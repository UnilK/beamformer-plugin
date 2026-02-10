#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/rbuffer.h"
#include "constants.h"
#include "BeamFormer.h"

#include <vector>
#include <random>

//==============================================================================

PluginAudioProcessor::PluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    state.micPositions = editorState.micPositions = createShowerFlowerArray(5, 4, PIF / 20.0f, 0.05f);
    startTimer(5);
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    stopTimer();
}

void PluginAudioProcessor::timerCallback(){
    lock.enter();
    state = editorState;
    editorFstate = fstate;
    lock.exit();
}

//==============================================================================
const juce::String PluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

static float rnd(const float &d){
    static std::mt19937 rng32;
    return std::uniform_real_distribution<float>(-d, d)(rng32);
}

void PluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ignoreUnused (buffer);

    juce::ScopedNoDenormals noDenormals;

    lock.enter();
    State newState = state;
    static State oldState = state;
    lock.exit();

    auto fsa = fstate;

    float fs = (float)this->getSampleRate();
    int mics = (int)newState.micPositions.size();

    ///////////////////////////////////////////////////////////////////////////
    // Simulate the propagation of sound from the target to the microphone array.
    ///////////////////////////////////////////////////////////////////////////

    fsa.fs = fs;
    int maxs = (int)(newState.maxd / fsa.c * fs) + 1;
    static std::vector<rbuffer<float> > micSamples(mics);

    constexpr int N = 12;

    micSamples.resize(mics);
    for(auto &r : micSamples){
        r.resize(maxs+3*N, 0.0f);
        r.set_offset(N);
    }

    constexpr float pin = (float)(PIF / N);
    auto sinc = [&](float x){ return x*x < 1e-12f ? 1.0f : sin(x * PIF) / (x * PIF); };
    auto impulse = [&](float x){ return (0.5f + 0.5f * cos(x * pin)) * sinc(x); };
    auto write_sample = [&](float *x, float p, float s, float d){
        int l = (int)std::ceil(p-N/d);
        int r = (int)std::floor(p+N/d);
        for(int i=l; i<=r; i++) x[i] += s * impulse((i-p)*d);
    };

    // Generate target noise
    int blocksize = buffer.getNumSamples();
    static std::vector<float> targetSamples;
    targetSamples.resize(blocksize);
    for(float &i : targetSamples) i = 0.0f;

    if(newState.targetNoise) for(float& i : targetSamples) i += rnd(1.0f);
    if(newState.targetSine){
        static float p = 0.0f;
        float angv = 2 * PIF * newState.targetFrequency / fs;
        for(float& i : targetSamples){
            i += std::sin(p);
            p = std::fmodf(p + angv, 2 * PIF);
        }
    }

    static std::vector<rbuffer<float> > outSamples(2);
    static std::vector<vec3> outPositions{{0, -0.1f, 0}, {0, 0.1f, 0}};
    for(auto &r : outSamples){
        r.resize(maxs+3*N, 0.0f);
        r.set_offset(N);
    }

    auto simulateSoundPropagation = [&](int sampleIndex, const std::vector<vec3>& micPos, std::vector<rbuffer<float> > &outputBuffer){
        float d = sampleIndex / (float)blocksize;
        vec3 targetPos = oldState.targetPosition * (1.0f - d) + newState.targetPosition * d;

        float dt = 1.0f / fs;
        float speed = ((targetPos + (newState.targetPosition - newState.targetPosition) * dt).abs() - targetPos.abs()) / dt;
        float dopplerCoeff = fsa.c / (speed + fsa.c);

        for(auto &r : outputBuffer) r.push(0);
        for(int j=0; j<(int)outputBuffer.size(); j++){
            float dist = std::min(newState.maxd, (micPos[j]-targetPos).abs());
            write_sample(&outputBuffer[j][0], dist / fsa.c * fs, targetSamples[sampleIndex] / (1.0f + dist), dopplerCoeff);
        }
    };

    for(int sampleIndex=0; sampleIndex<blocksize; sampleIndex++){
        simulateSoundPropagation(sampleIndex, outPositions, outSamples);
        for(int j=0; j<std::min<int>(2, buffer.getNumChannels()); j++){
            buffer.getWritePointer(j)[sampleIndex] = outSamples[j][0];
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Compute the filter phases for the beamformer visualization.
    ///////////////////////////////////////////////////////////////////////////

    // construct filter coefficients

    auto toFilterCoeff = [](float centerFrequency, float relativeHalfTime){
        return std::polar(std::pow(0.5f, 1.0f / (2 * PIF * relativeHalfTime / centerFrequency)), centerFrequency);
    };
    
    auto [angularFreq, delay] = createFilterBank(fs, 500, 2);
    fsa.micAngularFrequencies = angularFreq;
    fsa.micRelativeDecays = delay;

    int filters = (int)angularFreq.size();
    std::vector<std::complex<float> > filterCoeff(filters);
    std::vector<float> filterNorm(filters);
    for(int i=0; i<filters; i++){
        filterCoeff[i] = toFilterCoeff(angularFreq[i], delay[i]);
        filterNorm[i] = 1.0f - std::abs(filterCoeff[i]);
    }

    // Prepare filter array
    std::vector<std::complex<float> > currentAngularVelocity(filters, 0.0f);
    fsa.currentFilterPhases.resize(filters);
    fsa.averageFilterPhases.resize(filters);
    fsa.averageAngularVelocity.resize(filters);
    for(auto &i : fsa.currentFilterPhases) i.resize(mics);
    for(auto &i : fsa.averageFilterPhases) i.resize(mics);


    float avgCoeff = std::pow(newState.frameDecayRate,  newState.fps / fs);

    for(int sampleIndex=0; sampleIndex<blocksize; sampleIndex++){
        simulateSoundPropagation(sampleIndex, newState.micPositions, micSamples);

        for(auto& i : currentAngularVelocity) i = 0.0f;

        // compute filter phases and
        // estimate angular velocities for each filter by taking average over microphones.
        for(int j=0; j<mics; j++){
            for(int i=0; i<filters; i++){
                auto &s = fsa.currentFilterPhases[i][j];
                auto prev = s;
                s = s * std::conj(filterCoeff[i]) + filterNorm[i] * micSamples[j][0];
                currentAngularVelocity[i] += s * std::conj(prev);
            }
        }

        // update averaged filter phases
        for(int i=0; i<filters; i++){
            auto av = currentAngularVelocity[i] / (std::abs(currentAngularVelocity[i]) + 1e-18f);
            for(int j=0; j<mics; j++){
                fsa.averageFilterPhases[i][j] =
                    fsa.currentFilterPhases[i][j] * (1.0f - avgCoeff) +
                    fsa.averageFilterPhases[i][j] * av * avgCoeff;
            }
        }

        // update average angular velocities
        for(int i=0; i<filters; i++){
            fsa.averageAngularVelocity[i] =
                currentAngularVelocity[i] * (1.0f - avgCoeff) +
                fsa.averageAngularVelocity[i] * avgCoeff;

            // fsa.averageAngularVelocity[i] = std::conj(filterCoeff[i]);
        }
    }

    lock.enter();
    fstate = fsa;
    lock.exit();

    oldState = newState;
}

//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
