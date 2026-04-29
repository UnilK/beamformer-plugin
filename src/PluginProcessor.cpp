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
    processorState.micPositions = editorState.micPositions = createShowerFlowerArray(5, 4, PIF / 20.0f, 0.05f);
    startTimer(5);
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    stopTimer();
}

void PluginAudioProcessor::timerCallback(){
    lock.enter();
    processorState = editorState;
    editorFstate = processorFstate;
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
    State state = processorState;
    lock.exit();

    auto fstate = processorFstate;

    float fs = (float)this->getSampleRate();
    int mics = (int)state.micPositions.size();

    ///////////////////////////////////////////////////////////////////////////
    // Simulate the propagation of sound from the target to the microphone array.
    ///////////////////////////////////////////////////////////////////////////

    fstate.fs = fs;
    int maxs = (int)(state.maxd / fstate.c * fs) + 1;
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

    if(state.targetNoise) for(float& i : targetSamples) i += rnd(1.0f);
    if(state.targetSine){
        static float p = 0.0f;
        float angv = 2 * PIF * state.targetFrequency / fs;
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

    static std::vector<vec3> targetPositions;
    targetPositions.resize(blocksize, state.targetPosition);
    targetPositions[0] = targetPositions[blocksize-1];
    {
        float d = std::pow(0.5f, 1.0f / (fs * 0.05f));
        for(int i=1; i<blocksize; i++){
            targetPositions[i] = targetPositions[i-1] * d + state.targetPosition * (1.0f - d);
        }
        editorState.outTargetPosition = targetPositions[blocksize-1];
    }
    auto simulateSoundPropagation = [&](int sampleIndex, const std::vector<vec3>& micPos, std::vector<rbuffer<float> > &outputBuffer){

        const vec3 &targetPos = targetPositions[sampleIndex];

        float dt = 1.0f / fs;
        float speed = ((targetPos + (state.targetPosition - state.targetPosition) * dt).abs() - targetPos.abs()) / dt;
        float dopplerCoeff = fstate.c / (speed + fstate.c);

        for(auto &r : outputBuffer) r.push(0);
        for(int j=0; j<(int)outputBuffer.size(); j++){
            float dist = std::min(state.maxd, (micPos[j]-targetPos).abs());
            float sample = (targetSamples[sampleIndex] + rnd(state.nsr)) / (1.0f + dist);
            write_sample(&outputBuffer[j][0], dist / fstate.c * fs, sample, dopplerCoeff);
        }
    };

    for(int sampleIndex=0; sampleIndex<blocksize; sampleIndex++){
        simulateSoundPropagation(sampleIndex, outPositions, outSamples);
        float gain = (float)std::pow(10, state.outVolumedB / 20);
        for(int j=0; j<std::min<int>(2, buffer.getNumChannels()); j++){
            buffer.getWritePointer(j)[sampleIndex] = outSamples[j][0] * gain;
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
    fstate.micAngularFrequencies = angularFreq;
    fstate.micRelativeDecays = delay;

    int filters = (int)angularFreq.size();
    std::vector<std::complex<float> > filterCoeff(filters);
    std::vector<float> filterNorm(filters);
    for(int i=0; i<filters; i++){
        filterCoeff[i] = toFilterCoeff(angularFreq[i], delay[i]);
        filterNorm[i] = 1.0f - std::abs(filterCoeff[i]);
    }

    // Prepare filter array
    std::vector<std::complex<float> > currentAngularVelocity(filters, 0.0f);
    fstate.currentFilterPhases.resize(filters);
    fstate.averageFilterPhases.resize(filters);
    fstate.averageAngularVelocity.resize(filters);
    for(auto &i : fstate.currentFilterPhases) i.resize(mics);
    for(auto &i : fstate.averageFilterPhases) i.resize(mics);


    float avgCoeff = std::pow(state.frameDecayRate,  state.fps / fs);

    for(int sampleIndex=0; sampleIndex<blocksize; sampleIndex++){
        simulateSoundPropagation(sampleIndex, state.micPositions, micSamples);

        for(auto& i : currentAngularVelocity) i = 0.0f;

        // compute filter phases and
        // estimate angular velocities for each filter by taking average over microphones.
        for(int j=0; j<mics; j++){
            for(int i=0; i<filters; i++){
                auto &s = fstate.currentFilterPhases[i][j];
                auto prev = s;
                s = s * std::conj(filterCoeff[i]) + filterNorm[i] * micSamples[j][0];
                currentAngularVelocity[i] += s * std::conj(prev);
            }
        }

        // update averaged filter phases
        for(int i=0; i<filters; i++){
            auto av = currentAngularVelocity[i] / (std::abs(currentAngularVelocity[i]) + 1e-18f);
            for(int j=0; j<mics; j++){
                fstate.averageFilterPhases[i][j] =
                    fstate.currentFilterPhases[i][j] * (1.0f - avgCoeff) +
                    fstate.averageFilterPhases[i][j] * av * avgCoeff;
            }
        }

        // update average angular velocities
        for(int i=0; i<filters; i++){
            fstate.averageAngularVelocity[i] =
                currentAngularVelocity[i] * (1.0f - avgCoeff) +
                fstate.averageAngularVelocity[i] * avgCoeff;
        }

        if(state.disableFrequencyTracking){
            fstate.averageAngularVelocity = filterCoeff;
            for(auto &i : fstate.averageAngularVelocity) i = std::conj(i);
        }

        if(state.disablePhaseAveraging){
            fstate.averageFilterPhases = fstate.currentFilterPhases;
        }
    }

    lock.enter();
    processorFstate = fstate;
    lock.exit();
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
