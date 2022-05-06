/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthStarter.h"


//==============================================================================
MidiAttemptAudioProcessor::MidiAttemptAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif

    // the section below initialises all the parameters that will be available in the final plug-in

    parameters(*this, nullptr, "ParamTree", {

    //ParameterLayout, id, description,min,max,default

        std::make_unique<juce::AudioParameterFloat>("masterGain", "Organ/SynthesiserVolume", 0.001f, 10.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("sequencerGain", "SequencerVolume", 0.001f, 10.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("PanControlOrgan", "PanControlOrgan", 0.001f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("PanControlOrganFreq", "PanningLFOFrequencyOrgan", 0.001f, 3.0f, 0.0f),

        std::make_unique<juce::AudioParameterFloat>("PanControlSequencer", "PanControlSequencer", 0.001f, 1.0f, 0.5f),

        std::make_unique<juce::AudioParameterFloat>( "dry", "ReverbDryLevel",0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>( "wet", "ReverbWetLevel",0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>( "room","ReverbRoomSize", 0.0f, 1.0f, 0.5f),


        std::make_unique<juce::AudioParameterInt>("midiCutOff", "KeyboardSplitNote", 0, 120, 60),

        std::make_unique<juce::AudioParameterChoice>("globalftype", "GlobalFilterType", juce::StringArray{"Lowpass","HighPass","BandPass","NoFilter"},3),
        std::make_unique<juce::AudioParameterFloat>("globalcutoff", "GlobalCutoffFreq", 0.0001f, 20000.0f, 2.0f),
        std::make_unique<juce::AudioParameterFloat>("globalreso", "GlobalFilterResonance", 0.0001f, 20.0f, 2.0f),

        std::make_unique<juce::AudioParameterFloat>("globalLFOFreq", "GlobalFilterLFOFrequency", 0.0f, 3.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("globalLFOGain", "GlobalFilterLFOAmplitude", 0.0001f, 20000.0f, 5000.0f),

        std::make_unique<juce::AudioParameterFloat>("globalattack_filter", "GlobalFilterAttack", 0.001f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("globaldecay_filter", "GlobalFilterDecay", 0.001f, 2.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("globalsustain_filter", "GlobalFilterSustain", 0.001f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("globalrelease_filter", "GlobalFilterRelease", 0.001f, 2.0f, 0.3f),

        std::make_unique<juce::AudioParameterChoice>("type", "Waveform", juce::StringArray{"Phasor","Sine","Triangle","Square"},1),
        std::make_unique<juce::AudioParameterFloat>("pulse", "PulseWidth", 0.0001f, 1.0f, 0.3f),

        std::make_unique<juce::AudioParameterFloat>("detunefund", "FundamentalFreqDetune", 0.0f, 0.01f, 0.0f),

        std::make_unique<juce::AudioParameterFloat>("tremDepth", "TremoloDepth", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("tremRate", "TremoloRate", 0.0f, 10.0f, 0.01f),
        std::make_unique<juce::AudioParameterFloat>("tremPW", "TremoloPulseWidth", 0.0f, 1.0f, 0.5f),


        std::make_unique<juce::AudioParameterChoice>("tonality", "Tonality", juce::StringArray{"Minor", "Major"},1),
        std::make_unique<juce::AudioParameterFloat>("gain0", "16'", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain1", "5_1/3'", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain2", "8'", 0.0f, 2.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("gain3", "4'", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain4", "2_2/3''", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain5", "2'", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain6", "1_3/5''", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain7", "1_1/3'", 0.0f, 1.0f, 0.0001f),
        std::make_unique<juce::AudioParameterFloat>("gain8", "1'", 0.0f, 1.0f, 0.0001f),


        std::make_unique<juce::AudioParameterFloat>("attack_time", "Attack", 0.001f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("decay_time", "Decay", 0.001f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("sustain_time", "Sustain", 0.001f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("release_time", "Release", 0.001f, 5.0f, 0.3f),

        std::make_unique<juce::AudioParameterChoice>("ftype", "FilterType", juce::StringArray{"Lowpass","HighPass","BandPass","NoFilter"},3),
        std::make_unique<juce::AudioParameterFloat>("cutoff", "CutoffFreq", 0.0001f, 20.0f, 2.0f),
        std::make_unique<juce::AudioParameterFloat>("reso", "FilterResonance", 0.0001f, 20.0f, 2.0f),

        std::make_unique<juce::AudioParameterFloat>("attack_filter", "FilterAttack", 0.001f, 5.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("decay_filter", "FilterDecay", 0.001f, 5.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("sustain_filter", "FilterSustain", 0.001f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("release_filter", "FilterRelease", 0.001f, 5.0f, 0.3f),

        std::make_unique<juce::AudioParameterFloat>("lfoFreq", "FilterLFOFrequency", 0.0f, 2.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("lfoGain", "FilterLFOAmplitude", 0.0001f, 1.0f, 1.0f),

        std::make_unique<juce::AudioParameterChoice>("waveform", "SequencerWaveform", juce::StringArray{"Phasor","Sine","Triangle","Square"},1),
        std::make_unique<juce::AudioParameterChoice>("tonalitySequencer", "ScaleforSequencer", juce::StringArray{"Melodic Minor", "Melodic Major", "Mixolydian Major", "Phyrgian Major","Locrian Mode"},1),
        std::make_unique<juce::AudioParameterChoice>("Harmoniser", "Harmoniser", juce::StringArray{"Off","On"},0),

        std::make_unique<juce::AudioParameterFloat>("detune", "Detune", 0.0f, 0.03f, 0.01f),
        std::make_unique<juce::AudioParameterFloat>("tempo", "SetTempo", 0.0001f, 200.0f, 60.0f),
        std::make_unique<juce::AudioParameterInt>("timeSig", "Beats_Per_Bar", 0, 15, 4),
        std::make_unique<juce::AudioParameterInt>("npb", "NotesPerBeat", 0, 10, 2),

        std::make_unique<juce::AudioParameterChoice>("ftypeSequencer", "FilterTypeSequencer", juce::StringArray{"Lowpass","HighPass","BandPass","NoFilter"},3),
        std::make_unique<juce::AudioParameterFloat>("cutoffSequencer", "CutoffFreqSequencer", 0.0f, 10.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("resoSequencer", "FilterResonanceSequencer", 0.0001f, 20.0f, 2.0f),

        std::make_unique<juce::AudioParameterFloat>("attack_timeSequencer", "SequencerAttack", 0.001f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("decay_timeSequencer", "SequencerDecay", 0.001f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("sustain_timeSequencer", "SequencerSustain", 0.001f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("release_timeSequencer", "SequencerRelease", 0.001f, 1.0f, 0.3f),

        std::make_unique<juce::AudioParameterFloat>("lfoFreqSeq", "FilterLFOFrequencySeq", 0.0f, 2.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("lfoGainSeq", "FilterLFOAmplitudeSeq", 0.0001f, 1.0, 1.0f),
        })

{
    // ===========================================================================================================

    // the section of code below assigns the intialised pointers to the corresponding parameter

    globalAttack = parameters.getRawParameterValue("globalattack_filter");
    globalDecay = parameters.getRawParameterValue("globaldecay_filter");
    globalSustain = parameters.getRawParameterValue("globalsustain_filter");
    globalRelease = parameters.getRawParameterValue("globalrelease_filter");

    globalFilterType = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("globalftype"));
    globalFilterCutoff = parameters.getRawParameterValue("globalcutoff");
    globalFilterResonance = parameters.getRawParameterValue("globalreso");

    lfoFreqGlobal = parameters.getRawParameterValue("globalLFOFreq");
    lfoGainGlobal = parameters.getRawParameterValue("globalLFOGain");

    fundamentalDetuneParam = parameters.getRawParameterValue("detunefund");

    tremoloDepth = parameters.getRawParameterValue("tremDepth");
    tremoloRate = parameters.getRawParameterValue("tremRate");
    tremoloPulseWidth = parameters.getRawParameterValue("tremPW");

    attackParam = parameters.getRawParameterValue("attack_time");
    decayParam = parameters.getRawParameterValue("decay_time");
    sustainParam = parameters.getRawParameterValue("sustain_time");
    releaseParam = parameters.getRawParameterValue("release_time");

    masterGain = parameters.getRawParameterValue("masterGain");
    sequencerGain = parameters.getRawParameterValue("sequencerGain");

    panning = parameters.getRawParameterValue("PanControlOrgan");
    panningFreq = parameters.getRawParameterValue("PanControlOrganFreq");
    panningSeq = parameters.getRawParameterValue("PanControlSequencer");

    dryLevel = parameters.getRawParameterValue("dry");
    wetLevel = parameters.getRawParameterValue("wet");
    roomSize = parameters.getRawParameterValue("room");


    tonalityParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("tonality"));
    gain0 = parameters.getRawParameterValue("gain0");
    gain1 = parameters.getRawParameterValue("gain1");
    gain2 = parameters.getRawParameterValue("gain2");
    gain3 = parameters.getRawParameterValue("gain3");
    gain4 = parameters.getRawParameterValue("gain4");
    gain5 = parameters.getRawParameterValue("gain5");
    gain6 = parameters.getRawParameterValue("gain6");
    gain7 = parameters.getRawParameterValue("gain7");
    gain8 = parameters.getRawParameterValue("gain8");
    

    detuneParam = parameters.getRawParameterValue("detune");


    attackFilter = parameters.getRawParameterValue("attack_time");
    decayFilter = parameters.getRawParameterValue("decay_filter");
    sustainFilter = parameters.getRawParameterValue("sustain_filter");
    releaseFilter = parameters.getRawParameterValue("release_filter");


    pulseWidthParam = parameters.getRawParameterValue("pulse");
    waveformParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("type"));
    filterTypeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("ftype"));
    cutoffFreqParam = parameters.getRawParameterValue("cutoff");
    resonanceParam = parameters.getRawParameterValue("reso");

    lfoFreq = parameters.getRawParameterValue("lfoFreq");
    lfoGain = parameters.getRawParameterValue("lfoGain");

    midiCutOff = dynamic_cast<juce::AudioParameterInt*>(parameters.getParameter("midiCutOff"));
    tempo = parameters.getRawParameterValue("tempo");
    notesPerBeat = dynamic_cast<juce::AudioParameterInt*>(parameters.getParameter("npb"));
    beatsPerBar = dynamic_cast<juce::AudioParameterInt*>(parameters.getParameter("timeSig"));
    waveformParamSequencer = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("waveform"));
    tonalityParamSequencer = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("tonalitySequencer"));


    filterTypeParamSequencer = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("ftypeSequencer"));
    cutoffFreqParamSequencer = parameters.getRawParameterValue("cutoffSequencer");
    resonanceParamSequencer = parameters.getRawParameterValue("resoSequencer");

    attackParamSequencer = parameters.getRawParameterValue("attack_timeSequencer");
    decayParamSequencer = parameters.getRawParameterValue("decay_timeSequencer");
    sustainParamSequencer = parameters.getRawParameterValue("sustain_timeSequencer");
    releaseParamSequencer = parameters.getRawParameterValue("release_timeSequencer");

    lfoFreqSeq = parameters.getRawParameterValue("lfoFreqSeq");
    lfoGainSeq = parameters.getRawParameterValue("lfoGainSeq");

    harmony = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("Harmoniser"));

    //============================================================================================================================================

    // generates a new voice 
    for (int i = 0; i < voiceCount; i++)
    {
        synth.addVoice(new MySynthVoice());

    }

    for (int i = 0; i < voiceCount; i++)
    {
        synth.addVoice(new SequencerVoice());
    }

    // creates a new instance of the synth sound class
    synth.addSound(new MySynthSound());
    synth.addSound(new SequencerSound());
    //SequencerSound* s = dynamic_cast <SequencerSound*>(synth.getSound(1));
    //s->setMidiSplit(60);


    // the code below creates an instance of my synth, and links the pointers that correspond to envelope parameters
    for (int i = 0; i < voiceCount; i++)
    {
        MySynthVoice* v = dynamic_cast <MySynthVoice*>(synth.getVoice(i));
        v->linkEnvParameter(attackParam, decayParam, sustainParam, releaseParam, attackFilter, decayFilter, sustainFilter, releaseFilter
                            ,globalAttack,globalDecay,globalSustain,globalRelease);
    }

    for (int i = voiceCount; i < 2*voiceCount; i++)
    {
        SequencerVoice* v = dynamic_cast <SequencerVoice*>(synth.getVoice(i));
        v->linkEnvParam(attackParamSequencer, decayParamSequencer, sustainParamSequencer, releaseParamSequencer, globalAttack, globalDecay, globalSustain, globalRelease);
    }
}

MidiAttemptAudioProcessor::~MidiAttemptAudioProcessor()
{
}

//==============================================================================
const juce::String MidiAttemptAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidiAttemptAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidiAttemptAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidiAttemptAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidiAttemptAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiAttemptAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidiAttemptAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidiAttemptAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidiAttemptAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidiAttemptAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MidiAttemptAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    

    // sets the sample rate of the final plug-in
    synth.setCurrentPlaybackSampleRate(sampleRate);
    reverb.setSampleRate(sampleRate);

    for (int i = 0; i < voiceCount; i++)
    {
        MySynthVoice* v = dynamic_cast <MySynthVoice*>(synth.getVoice(i));
        v->setSampleRate(sampleRate);
    }

    for (int i = voiceCount; i < 2*voiceCount; i++)
    {
        SequencerVoice* v = dynamic_cast <SequencerVoice*>(synth.getVoice(i));
        v->setSampleRate(sampleRate);
    }

    SR = sampleRate;    // stores the sample rate as a member variable
}

void MidiAttemptAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiAttemptAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void MidiAttemptAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();

    for (int i = 0; i < voiceCount; i++)
    {
        MySynthVoice* v = dynamic_cast <MySynthVoice*>(synth.getVoice(i));

        // links all the plug-in parameters with corresponding variables in each voice
        v->setParameters(*waveformParam, *pulseWidthParam, *filterTypeParam, *cutoffFreqParam, *resonanceParam, *lfoFreq, *lfoGain, *masterGain,*detuneParam,*tonalityParam
                        ,gain0, gain1,gain2,gain3, gain4, gain5, gain6, gain7, gain8,  *panning, *globalFilterType, *globalFilterCutoff,*globalFilterResonance, *lfoFreqGlobal, 
                        *lfoGainGlobal, *fundamentalDetuneParam, *tremoloDepth, *tremoloPulseWidth, *tremoloRate, *panningFreq);
    }

    for (int i = voiceCount; i < 2 * voiceCount; i++)
    {
        SequencerVoice* v = dynamic_cast <SequencerVoice*>(synth.getVoice(i));
        v->linkParameters(*waveformParamSequencer, *tempo, *notesPerBeat, *midiCutOff, *tonalityParamSequencer, *beatsPerBar
            , *filterTypeParamSequencer, *resonanceParamSequencer, *cutoffFreqParamSequencer, *sequencerGain, *lfoFreqSeq, *lfoGainSeq, *harmony,
            *panningSeq, *detuneParam, attackParamSequencer, decayParamSequencer, sustainParamSequencer, releaseParamSequencer, *lfoFreqGlobal,
            *lfoGainGlobal, *globalFilterResonance, *globalFilterCutoff, *globalFilterType);
    }



    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
    juce::Reverb::Parameters reverbParams;
    reverbParams.dryLevel = *dryLevel;
    reverbParams.wetLevel = *wetLevel;
    reverbParams.roomSize = *roomSize;

    reverb.setParameters(reverbParams);

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);



    delay.setBufferSize(SR);
    delay.setFeedback(0.1);
    delay.setDelayTime(SR * 0.5f);

    reverb.processStereo(leftChannel, rightChannel, numSamples);


    
        // ..do something to the data...
    }

//==============================================================================
bool MidiAttemptAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiAttemptAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void MidiAttemptAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MidiAttemptAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiAttemptAudioProcessor();
}
