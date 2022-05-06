/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthStarter.h"
#include "Oscillator.h"
#include "Sequencer.h"
#include "Filter.h"
#include "DelayLine.h"
//==============================================================================
/**
*/
class MidiAttemptAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MidiAttemptAudioProcessor();
    ~MidiAttemptAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================


    float SR;
    juce::Synthesiser synth;                            // initialises an instance of the juce synth class
    juce::Synthesiser synth1;

    std::atomic<float>* masterGain; 

    juce::AudioProcessorValueTreeState parameters;           // initialises a valuetreestate to store plug-in parameters
    std::atomic<float>* attackParam;                         // initialises a parameter to control the attack phase of an envelope   
    std::atomic<float>* decayParam;                          // initialises a parameter to control the decay phase of an envelope
    std::atomic<float>* sustainParam;                        // initialises a parameter to control the sustain phase of an envelope
    std::atomic<float>* releaseParam;                        // initialises a parameter to control the release phase of an envelope
         
    std::atomic<float>* detuneParam;                         // initialises a paramater that will control the detuning of oscillators
    std::atomic<float>* fundamentalDetuneParam;              // initialises a paramater that will control the detuning of oscillators

    std::atomic<float>* attackFilter;                        // initialises a parameter to control the attack phase of an envelope
    std::atomic<float>* decayFilter;                         // initialises a parameter to control the decay phase of an envelope
    std::atomic<float>* sustainFilter;                       // initialises a parameter to control the sustain phase of an envelope
    std::atomic<float>* releaseFilter;                       // initialises a parameter to control the release phase of an envelope

    juce::AudioParameterChoice* waveformParam;               // initialises a parameter that will control the waveform used for a voice
    std::atomic<float>* pulseWidthParam;                     // initialises a parameter that will control the pulsewidth of a squarewave


    juce::AudioParameterChoice* filterTypeParam;             // initialises a parameter that will control the type of filter applied
    std::atomic<float>* cutoffFreqParam;                     // initialises a parameter to control the cutoff frequency of a filter
    std::atomic<float>* resonanceParam;                      // initialises a paramater that will control the resonant frequency of a filter


    std::atomic<float>* lfoFreq;                             // controls the frequency of an LFO
    std::atomic<float>* lfoGain;                             // controls the gain of an LFO


    int voiceCount = 20;                                     // initialises the number of voices that can be played at once


    //===================================================================

    juce::AudioParameterChoice* tonalityParam;              // initialises a parameter that will control the tonality of the drawbars 


    // the code below intialises 9 gain parameters that will control different frequencies
    std::atomic<float>* gain0;
    std::atomic<float>* gain1;
    std::atomic<float>* gain2;
    std::atomic<float>* gain3;
    std::atomic<float>* gain4;
    std::atomic<float>* gain5;
    std::atomic<float>* gain6;
    std::atomic<float>* gain7;
    std::atomic<float>* gain8;

    std::atomic<float>* panning;                             // initialises a parameter that will control panning of a voice
    std::atomic<float>* panningFreq;

    //===================================================================

    std::atomic<float>* panningSeq;                         // initialises a parameter that will control panning of a voice



    juce::AudioParameterInt* midiCutOff;                    // initialises the split posiiton on the midi keyboard
    juce::AudioParameterInt* notesPerBeat;                  // initialises a parameter controlling the speed of the sequencer

    juce::AudioParameterInt* beatsPerBar;


    juce::AudioParameterChoice* waveformParamSequencer;     // initialises a parameter that will control the waveform used for a voice     
    juce::AudioParameterChoice* tonalityParamSequencer;     // this parameter will control the scale of notes the sequencer selects from


    juce::AudioParameterChoice* filterTypeParamSequencer;   // initialises a parameter that will control the type of filter applied
    std::atomic<float>* cutoffFreqParamSequencer;           // initialises a parameter to control the cutoff frequency of a filter
    std::atomic<float>* resonanceParamSequencer;            // initialises a parameter to control the resonance of a filter


    std::atomic<float>* sequencerGain;                      // gain parameter for the sequencer voice

    std::atomic<float>* attackParamSequencer;               // initialises a parameter to control the attack phase of an envelope
    std::atomic<float>* decayParamSequencer;                // initialises a parameter to control the decay phase of an envelope
    std::atomic<float>* sustainParamSequencer;              // initialises a parameter to control the sustain phase of an envelope
    std::atomic<float>* releaseParamSequencer;              // initialises a parameter to control the release phase of an envelope

    std::atomic<float>* lfoFreqSeq;                         // initialises a parameter that will control the frequency of the LFO altering the cutoff frequency of a filter
    std::atomic<float>* lfoGainSeq;                         // initialises a parameter controlling the amplitude of an LFO 

    juce::AudioParameterChoice* harmony;                    // acts as a binary parameter to select whether the sequencer can harmonise


    std::atomic<float>* tempo;

    juce::AudioParameterChoice* globalFilterType;           // initialises a parameter that will control the type of filter applied
    std::atomic<float>* globalFilterCutoff;                 // initialises a parameter to control the cutoff frequency of a filter
    std::atomic<float>* globalFilterResonance;              // initialises a parameter to control the resonance of a filter
    

    std::atomic<float>* globalAttack;                       // initialises a parameter to control the attack phase of an envelope
    std::atomic<float>* globalDecay;                        // initialises a parameter to control the decay phase of an envelope
    std::atomic<float>* globalSustain;                      // initialises a parameter to control the sustain phase of an envelope
    std::atomic<float>* globalRelease;                      // initialises a parameter to control the release phase of an envelope

    std::atomic<float>* lfoFreqGlobal;                      // initialises a parameter that will control the frequency of the LFO altering the cutoff frequency of a filter
    std::atomic<float>* lfoGainGlobal;                      // initialises a parameter controlling the amplitude of an LFO 


    //===========================================================================================


    
    juce::Reverb reverb;
    std::atomic<float>* dryLevel;
    std::atomic<float>* wetLevel;
    std::atomic<float>* roomSize;


    std::atomic<float>* tremoloDepth;                       // initialises parameter to control tremolo depth
    std::atomic<float>* tremoloRate;                        // initialises parameter to control tremolo rate
    std::atomic<float>* tremoloPulseWidth;                  // initialises parameter to store tremolo pulsewidth



    Delay delay;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiAttemptAudioProcessor)
};
