#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/rbuffer.h"

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
    startTimer(5);
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    stopTimer();
}

void PluginAudioProcessor::timerCallback(){
    lock.enter();
    state = editorState;
    editorMicf = micf;
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

    MicFilters mf = micf;
    int fs = (int)this->getSampleRate();
    int mics = (int)newState.micPositions.size();
    int maxs = (int)(newState.maxd / newState.c * fs) + 1;
    static std::vector<rbuffer<float> > micSamples(mics);

    int N = 12;

    micSamples.resize(mics);
    for(auto &r : micSamples){
        r.resize(maxs+3*N, 0.0f);
        r.set_offset(N);
    }

    const float pi = (float)(std::atan(1) * 4);
    const float pin = (float)(pi / N);
    auto sinc = [&](float x){ return x*x < 1e-12f ? 1.0f : sin(x * pi) / (x * pi); };
    auto impulse = [&](float x){ return (0.5f + 0.5f * cos(x * pin)) * sinc(x); };
    auto write_sample = [&](float *x, float p, float s){
        int l = (int)std::ceil(p-N);
        int r = (int)std::floor(p+N);
        for(int i=l; i<=r; i++) x[i] += s * impulse(i-p);
    };

    static std::vector<rbuffer<float> > outSamples(2);
    static std::vector<vec3> outPositions{{0, -0.1f, 0}, {0, 0.1f, 0}};
    for(auto &r : outSamples){
        r.resize(maxs+3*N, 0.0f);
        r.set_offset(N);
    }

    int n = buffer.getNumSamples();
    for(int i=0; i<n; i++){

        float d = i / (float)n;
        vec3 targetPos = oldState.targetPosition * (1.0f - d) + newState.targetPosition * d;
        float targetSample = rnd(1.0f);

        for(auto &r : outSamples) r.push(0);
        for(int j=0; j<2; j++){
            float dist = std::min(newState.maxd, (outPositions[j]-targetPos).abs());
            write_sample(&outSamples[j][0], dist / newState.c * fs, targetSample / (1.0f + dist));
        }

        for(int j=0; j<std::min<int>(2, buffer.getNumChannels()); j++){
            buffer.getWritePointer(j)[i] = outSamples[j][0];
        }
    }

    lock.enter();
    micf = mf;
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
