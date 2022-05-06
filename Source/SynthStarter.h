/*
  ==============================================================================

    YourSynthesiser.h
    Created: 7 Mar 2020 4:27:57pm
    Author:  Tom Mudd

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"
#include "Filter.h"
#include "Drawbars.h"
#include "Sequencer.h"



// ===========================
// ===========================
// SOUND


class SequencerSound : public juce::SynthesiserSound
{
public:
    bool appliesToChannel(int) override
    {
        return true;
    }

    //void setMidiSplit(int cutoff)
    //{
    //    midiSplit = cutoff;
    //}


    bool appliesToNote (int note) override
    {

        if (note < 60) { return true; }

        else { return false; }

    }

private:
    int midiSplit;

};
class MySynthSound : public juce::SynthesiserSound
{
public:
    //--------------------------------------------------------------------------
    bool appliesToChannel   (int) override      
    { 
        return true; 
    }
    bool appliesToNote      (int note) override      
    {
        if (note >= 60) { return true; }

        else { return false; }
    }
};


class SequencerVoice : public juce::SynthesiserVoice
{
public:
    SequencerVoice() {}


    /**
    * sets the sample rate of all elements in the elements in the voice that require it
    */
    void setSampleRate(float sampleRate)
    {
        sequencerEnv.setSampleRate(sampleRate);
        trial.setSampleRate(sampleRate);
        sequencerFilter.setSampleRate(sampleRate);
        lfoSeqFilter.setSampleRate(sampleRate);
        globalFilter.setSampleRate(sampleRate);
        globalFilterLFO.setSampleRate(sampleRate);


        // initialises all smoothed values
        smoothDetune.reset(sampleRate, 0.1f);
        smoothFilterCutoff.reset(sampleRate, 0.1f);
        smoothFilterReso.reset(sampleRate, 0.1f);
        smoothLeftGain.reset(sampleRate, 0.1f);
        smoothLFOFreq.reset(sampleRate, 0.1f);
        smoothLFOGain.reset(sampleRate, 0.1f);
        smoothRightGain.reset(sampleRate, 0.1f);
        smoothSequencerGain.reset(sampleRate, 0.1f);
        smoothGlobalLFOGain.reset(sampleRate, 0.1f);
        smoothGlobalLFOFreq.reset(sampleRate, 0.1f);
        smoothGlobalFilterReso.reset(sampleRate, 0.1f);
        smoothGlobalFilterCutoff.reset(sampleRate, 0.1f);

        smoothDetune.setCurrentAndTargetValue(0.0f);
        smoothFilterCutoff.setCurrentAndTargetValue(0.0f);
        smoothFilterReso.setCurrentAndTargetValue(0.0f);
        smoothLeftGain.setCurrentAndTargetValue(0.0f);
        smoothLFOFreq.setCurrentAndTargetValue(0.0f);
        smoothLFOGain.setCurrentAndTargetValue(0.0f);
        smoothRightGain.setCurrentAndTargetValue(0.0f);
        smoothSequencerGain.setCurrentAndTargetValue(0.0f);
        smoothGlobalLFOGain.setCurrentAndTargetValue(0.0f);
        smoothGlobalLFOFreq.setCurrentAndTargetValue(0.0f);
        smoothGlobalFilterReso.setCurrentAndTargetValue(0.0f);
        smoothGlobalFilterCutoff.setCurrentAndTargetValue(0.0f);
    }

    /**
    *   Links all parameters related to envelopes
    */
    void linkEnvParam(std::atomic<float>* ptrToAttackSequencer, std::atomic<float>* ptrToDecaySequencer, std::atomic<float>* ptrToSustainSequencer,
        std::atomic<float>* ptrToReleaseSequencer, std::atomic<float>* ptrToAttackSequencerGlobal, std::atomic<float>* ptrToDecaySequencerGlobal, std::atomic<float>* ptrToSustainSequencerGlobal,
        std::atomic<float>* ptrToReleaseSequencerGlobal)
    {
        attackSequencer = ptrToAttackSequencer;
        decaySequencer = ptrToDecaySequencer;
        sustainSequencer = ptrToSustainSequencer;
        releaseSequencer = ptrToReleaseSequencer;

        globalAttack = ptrToAttackSequencerGlobal;
        globalDecay = ptrToDecaySequencerGlobal;
        globalRelease = ptrToReleaseSequencerGlobal;
        globalSustain = ptrToSustainSequencerGlobal;
    }


    /**
    *   links all relevant paramters from the PluginProcessor to this voice
    */
    void linkParameters(int waveform, float _tempo, int _npb,
        int keyboardSplit, int tonalitySequencer, int timeSignature, int seqFilType, float seqFilRes, float seqFilCutoff, float seqGain, float lfoSeqFreq,
        float lfoSeqGain, int harmonyBinary, float right, float detune, std::atomic<float>* ptrToAttackSequencer, std::atomic<float>* ptrToDecaySequencer, std::atomic<float>* ptrToSustainSequencer,
        std::atomic<float>* ptrToReleaseSequencer, float globalFreqLFO, float globalGainLFO, float globalReso,  float globalFreq,float globalType)
    {
        waveformSequencer = waveform;
        tempo = _tempo;
        notesPerBeat = _npb;
        sequencerTonality = tonalitySequencer;
        timeSig = timeSignature;
        sequencerFilterType = seqFilType;
        sequencerFilterResonance = seqFilRes;
        sequencerCutoffFreq = seqFilCutoff;
        sequencerGain = seqGain;
        lfoFreqSeq = lfoSeqFreq;
        lfoGainSeq = lfoSeqGain;
        harmony = harmonyBinary;
        rightChannelGainSequencer = right;
        leftChannelGainSequencer = 1 - right;

        detuneParam = detune;

        globalResonance = globalReso;
        globalCutOff = globalFreq;
        globalLFOFrequency = globalFreqLFO;
        globalLFOGain = globalGainLFO;
        globalFilterType = globalType;
    }



    /**
    * This function is called as soon as a note is triggered. It is responsible for setting parameters that won't change as the note is held.
    */
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {

        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);           // stores the root note frequency
        trial.fillRatio(sequencerTonality);                                     // sets the scale the sequencer will choose from
        trial.setFreq(freq);                                                    // sets the root note frequency of the sequencer

        sampleCount = 0;                                                        // initialises sample count


        // sets the envelope parameters
        juce::ADSR::Parameters envParamsSeq;
        envParamsSeq.attack = *attackSequencer;
        envParamsSeq.decay = *decaySequencer;
        envParamsSeq.sustain = *sustainSequencer;
        envParamsSeq.release = *releaseSequencer;
        sequencerEnv.setParameters(envParamsSeq);    
        globalFilter.setEnvelope(globalAttack, globalDecay, globalSustain, globalRelease);      // sets the global filter envelope parameters


        sequencerEnv.reset();
        sequencerEnv.noteOn();
    }

    
    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
    Triggers the note off function of envelopes involved
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        sequencerEnv.noteOff();
        clearCurrentNote();
    }

    //--------------------------------------------------------------------------
    /**
    The code below does the bulk of the DSP. It calls all the relevant process functions of all classes involved,
    as well as setting key parameter values of all classes involved. It sets the sends the values to the buffer
    channels.
     */

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (sequencerEnv.isActive()) // check to see if this voice should be playing
        {
            // sets appropriate smoothed value target values
            smoothDetune.setTargetValue(detuneParam);
            smoothFilterCutoff.setTargetValue(sequencerCutoffFreq);
            smoothFilterReso.setTargetValue(sequencerFilterResonance);
            smoothLeftGain.setTargetValue(leftChannelGainSequencer);
            smoothLFOFreq.setTargetValue(lfoFreqSeq);
            smoothLFOGain.setTargetValue(lfoGainSeq);
            smoothRightGain.setTargetValue(rightChannelGainSequencer);
            smoothSequencerGain.setTargetValue(sequencerGain);
            smoothGlobalLFOFreq.setTargetValue(globalLFOFrequency);
            smoothGlobalLFOGain.setTargetValue(globalLFOGain);
            smoothGlobalFilterReso.setTargetValue(globalResonance);
            smoothGlobalFilterCutoff.setTargetValue(globalCutOff);

            
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {

                // calculates smoothed values next value
                float detuneParamVal = smoothDetune.getNextValue();
                float sequencerCutoffFreqVal = smoothFilterCutoff.getNextValue();
                float sequencerFilterResonanceVal = smoothFilterReso.getNextValue();
                float leftChannelGainSequencerVal = smoothLeftGain.getNextValue();
                float lfoFreqSeqVal = smoothLFOFreq.getNextValue();
                float lfoGainSeqVal = smoothLFOGain.getNextValue();
                float rightChannelGainSequencerVal = smoothRightGain.getNextValue();
                float sequencerGainVal = smoothSequencerGain.getNextValue();
                float globalLFOFreqVal = smoothGlobalLFOFreq.getNextValue();
                float globalLFOGainVal = smoothGlobalLFOGain.getNextValue();
                float globalCutoffVal = smoothGlobalFilterCutoff.getNextValue();
                float globalResonanceVal = smoothGlobalFilterReso.getNextValue();



                globalFilterLFO.setFrequency(globalLFOFreqVal);                                                             // sets LFO frequency determined by appropriate parameter
                float lfoFilter = globalFilterLFO.process(1, 0) * globalLFOGainVal;                                         // calculates the output value of the LFO

                globalFilter.setFilterType(globalFilterType);                                                               // sets the global filter type

                float cutoff = globalCutoffVal + lfoFilter;                                                                 // calculates the cutoff including the LFO value

                // the code below ensures the cutoff frequency is in an appropriate range
                if (cutoff <= 0.0f)
                    cutoff = 1.0f;
                else if (cutoff > 22000.0f)
                    cutoff = 20000.0f;

                globalFilter.setFrequencyEnvelope(cutoff);                                                                  // sets the cutoff frequency to the appropriate value
                globalFilter.setResonance(globalResonanceVal);                                                              // sets the filter resonance



                sampleCount += 1;
                lfoSeqFilter.setFrequency(lfoFreqSeqVal);                                                               // sets lfo frequency
                float lfoSeq = lfoSeqFilter.process(1, 0) * lfoGainSeqVal;                                              // caulcuates current sample of the lfo


                globalFilter.setFilterType(globalFilterType);                                                         // sets global filter type
                sequencerFilter.setFilterType(sequencerFilterType);                                                     // sets local filter type


                sequencerFilter.setFrequency(sequencerCutoffFreqVal * freq + (freq * lfoSeq * sequencerCutoffFreqVal) + 0.001f);   // sets local filter cutoff
                sequencerFilter.setResonance(sequencerFilterResonanceVal);                                                         // sets local filter resonance



                float envVal = sequencerEnv.getNextSample();                                                            // calculates next envelope value


                trial.setNotesPerBeat(notesPerBeat);                                                                    // sets the notes per beat
                trial.setTempo(tempo);                                                                                  // sets the tempo of the sequencer

                float sample = trial.process(waveformSequencer, sampleCount, detuneParam, harmony) * envVal;            //  calculates unfiltered sample and multiplies it by the current envelope value
                float currentSample = globalFilter.process(sequencerFilter.process(sample)) * sequencerGainVal;                               //  passes the sample through the local and global filters



                outputBuffer.addSample(0, sampleIndex, leftChannelGainSequencerVal * (currentSample));          //  passes sample into left channel, multiplying by the appropriate gain
                outputBuffer.addSample(1, sampleIndex, rightChannelGainSequencerVal * (currentSample));         //  passes sample into right channel, multiplying by the appropriate gain
                // for each channel, write the currentSample float to the output

            }
        }
    }
    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SequencerSound*> (sound) != nullptr;
    }

private:
    float freq;                                             // stores the fundamental frequency of the triggered note
    Sequencer trial;                                        // creates an instance of the sequencer class

    float sequencerGain;                                    // gain control

    juce::SmoothedValue<float> smoothSequencerGain;       // initialises a smoothed value to ensure clickless transitions

    int sampleCount;                                        // creates a variable that will count samples, starts at zero every time a note is pressed

    float tempo;                                            // controls the bpm of the sequencing
    int midiNoteCutoff;                 
    int notesPerBeat;                                       // controls how many notes per beat there are 
    int timeSig;
    int waveformSequencer;                                  // controls the waveform type
    int sequencerTonality;                                  // controls the available notes that the sequencer can select from


    Filter sequencerFilter;                                 // initialises a filter
    int sequencerFilterType;                                // controls local filter type
    float sequencerFilterResonance;                         // controls local filter resonance
    float sequencerCutoffFreq;                              // local filter cutoff frequency

    juce::SmoothedValue<float> smoothFilterCutoff;          // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothFilterReso;            // initialises a smoothed value to ensure clickless transitions

    float detuneParam;                                      // a detune parameter
    juce::SmoothedValue<float> smoothDetune;                // initialises a smoothed value to ensure clickless transitions


    // controls the envelope parameters of a gain ADSR envelope
    juce::ADSR sequencerEnv;
    std::atomic<float>* attackSequencer;
    std::atomic<float>* decaySequencer;
    std::atomic<float>* sustainSequencer;
    std::atomic<float>* releaseSequencer;


    Phasor lfoSeqFilter;                                    // initialises an lfo that will be linked to local filter cutoff
    float lfoFreqSeq;                                       // lfo frequency
    float lfoGainSeq;                                       // lfo amplitude

    juce::SmoothedValue<float> smoothLFOFreq;               // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothLFOGain;               // initialises a smoothed value to ensure clickless transitions


    int harmony;                                            // a binary variable that will control whether or not the sequencer includes harmonious notes



    float leftChannelGainSequencer;                         // left channel gain
    float rightChannelGainSequencer;                        // right channel gain

    juce::SmoothedValue<float> smoothRightGain;             // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothLeftGain;              // initialises a smoothed value to ensure clickless transitions


    //=================================================

    Filter globalFilter;                                    // creates another instance of the filter class, this acts on all voices present


    // controls envelope parameters for the global filter

    std::atomic<float>* globalAttack;
    std::atomic<float>* globalDecay;
    std::atomic<float>* globalSustain;
    std::atomic<float>* globalRelease;

    int globalFilterType;                                   // controls filter type
    float globalCutOff;                                     // controls filter cutoff frequency
    float globalResonance;                                  // controls filter resonance

    juce::SmoothedValue<float> smoothGlobalFilterCutoff;    // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGlobalFilterReso;      // initialises a smoothed value to ensure clickless transitions

    Phasor globalFilterLFO;                                 // creates an instance of the phasor class to be used as an LFO
    float globalLFOFrequency;                               // creates a variable that will be used to set the LFO frequency
    float globalLFOGain;                                    // controls the LFO gain


    juce::SmoothedValue<float> smoothGlobalLFOFreq;         // initialises a smoothed value to ensure clickless transitions         
    juce::SmoothedValue<float> smoothGlobalLFOGain;         // initialises a smoothed value to ensure clickless transitions

    //============================================

};


/**
* The class below handles the voice for the basic synthesiser/organ class. This will always be mapped to the right hand side of the keyboard split.
* With this class there are associated gain controls for harmonic frequencies, controls for a local filter (with an envelope), an ADSR gain envelope,
* LFO parameter controls and panning parameters.
*/
class MySynthVoice : public juce::SynthesiserVoice
{
public:
    MySynthVoice() {}
    //--------------------------------------------------------------------------
    /**
    * sets the sample rate of anything in the voice that requires a sample rate.
    */

    void setSampleRate(float sampleRate)
    {
        lfo.setSampleRate(sampleRate);
        filter.setSampleRate(sampleRate);
        env.setSampleRate(sampleRate);
        outputSound.fillOscillatorVector(sampleRate);
        SR = sampleRate;        // stores the sample rate
        globalFilter.setSampleRate(sampleRate);
        globalFilterLFO.setSampleRate(sampleRate);
        tremolo.setSampleRate(sampleRate);
        panningLFO.setSampleRate(sampleRate);

        // the block of code below does the required srtting up for smoothed values, setting the sample rate
        // and ramp length
        smoothCutoffFilter.reset(sampleRate, 0.1f);
        smoothFundDetune.reset(sampleRate, 0.1f);
        smoothGain0.reset(sampleRate, 0.1f);
        smoothGain1.reset(sampleRate, 0.1f);
        smoothGain2.reset(sampleRate, 0.1f);
        smoothGain3.reset(sampleRate, 0.1f);
        smoothGain4.reset(sampleRate, 0.1f);
        smoothGain5.reset(sampleRate, 0.1f);
        smoothGain6.reset(sampleRate, 0.1f);
        smoothGain7.reset(sampleRate, 0.1f);
        smoothGain8.reset(sampleRate, 0.1f);
        smoothGlobalCutoff.reset(sampleRate, 0.1f);
        smoothGlobalLFOFreq.reset(sampleRate, 0.1f);
        smoothGlobalLFOGain.reset(sampleRate, 0.1f);
        smoothGlobalResonance.reset(sampleRate, 0.1f);
        smoothLeftGain.reset(sampleRate, 0.1f);
        smoothLFOFreq.reset(sampleRate, 0.1f);
        smoothLFOGain.reset(sampleRate, 0.1f);
        smoothMasterGain.reset(sampleRate, 0.1f);
        smoothPulseWidth.reset(sampleRate, 0.1f);
        smoothResonanceFilter.reset(sampleRate, 0.1f);
        smoothRightGain.reset(sampleRate, 0.1f);
        smoothTremoloDepth.reset(sampleRate, 0.1f);
        smoothTremoloRate.reset(sampleRate, 0.1f);
        smoothTremoloPulseWidth.reset(sampleRate, 0.1f);


        smoothCutoffFilter.setCurrentAndTargetValue(0.0);
        smoothFundDetune.setCurrentAndTargetValue(0.0);
        smoothGain0.setCurrentAndTargetValue(0.0);
        smoothGain1.setCurrentAndTargetValue(0.0);
        smoothGain2.setCurrentAndTargetValue(0.0);
        smoothGain3.setCurrentAndTargetValue(0.0);
        smoothGain4.setCurrentAndTargetValue(0.0);
        smoothGain5.setCurrentAndTargetValue(0.0);
        smoothGain6.setCurrentAndTargetValue(0.0);
        smoothGain7.setCurrentAndTargetValue(0.0);
        smoothGain8.setCurrentAndTargetValue(0.0);
        smoothGlobalCutoff.setCurrentAndTargetValue(0.0);
        smoothGlobalLFOFreq.setCurrentAndTargetValue(0.0);
        smoothGlobalLFOGain.setCurrentAndTargetValue(0.0);
        smoothGlobalResonance.setCurrentAndTargetValue(0.0);
        smoothLeftGain.setCurrentAndTargetValue(0.0);
        smoothLFOFreq.setCurrentAndTargetValue(0.0);
        smoothLFOGain.setCurrentAndTargetValue(0.0);
        smoothMasterGain.setCurrentAndTargetValue(0.0);
        smoothPulseWidth.setCurrentAndTargetValue(0.0);
        smoothResonanceFilter.setCurrentAndTargetValue(0.0);
        smoothRightGain.setCurrentAndTargetValue(0.0);
        smoothTremoloDepth.setCurrentAndTargetValue(0.0);
        smoothTremoloRate.setCurrentAndTargetValue(0.0);
        smoothTremoloPulseWidth.setCurrentAndTargetValue(0.0);


    }


    /**
    * This function is called as soon as a note is triggered. It is responsible for setting parameters that won't change as the note is held.
    */
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {



        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);                           // calculates frequency from midi note
        outputSound.setFrequencyRatios(tonality);                                               // sets the tonality of the drawbar frequencies
        outputSound.setFrequencies(freq, fundamentalDetune);                                    // initialises list of frequencies 


        //  sets the envelope parameter values
        juce::ADSR::Parameters envParams;
        envParams.attack = *attackParam;
        envParams.decay = *decayParam;
        envParams.sustain = *sustainParam;
        envParams.release = *releaseParam;

        env.setParameters(envParams);

        env.reset();
        env.noteOn();

        filter.setEnvelope(attackFilter, decayFilter, sustainFilter, releaseFilter);            // sets the filter envelope parameters
        globalFilter.setEnvelope(globalAttack, globalDecay, globalSustain, globalRelease);      // sets the global filter envelope parameters

        //==========================================================================        
    }

    /**
    * Links the appropriate parameter values to be used in the current block of samples
    */
    void setParameters(int type, float pw, int filter, float cutoff, float reso, float lfofreq, float lfogain, float gain,float detune,int _tonality, 
        std::atomic<float>* _gain0, std::atomic<float>* _gain1, std::atomic<float>* _gain2, std::atomic<float>* _gain3, std::atomic<float>* _gain4, 
        std::atomic<float>* _gain5, std::atomic<float>* _gain6, std::atomic<float>* _gain7, std::atomic<float>* _gain8, float right, float globalType, float globalFreq, float globalReso, float globalLFOFreq, float globalGainLFO, float detuneFund,
        float depth, float pulse, float rate, float pan)
    {

        waveform = type;
        pulseWidth = pw;
        filterType = filter;
        cutoffFilter = cutoff;
        resonanceFilter = reso;
        lfoFreq = lfofreq;
        lfoGain = lfogain;
        masterGain = gain;
        tonality = _tonality;

        gain0 = *_gain0;
        gain1 = *_gain1;
        gain2 = *_gain2;
        gain3 = *_gain3;
        gain4 = *_gain4;
        gain5 = *_gain5;
        gain6 = *_gain6;
        gain7 = *_gain7;
        gain8 = *_gain8;

        rightChannelGain = right;
        leftChannelGain = (1 - right);

        globalFilterType = globalType;
        globalResonance = globalReso;
        globalCutOff = globalFreq;
        globalLFOFrequency = globalLFOFreq;
        globalLFOGain = globalGainLFO;

        fundamentalDetune = detuneFund;

        tremoloDepth = depth;
        tremoloRate = rate;
        tremoloPulseWidth = pulse;

        panningFreq = pan;
    }

    /**
    * This function links the pointer to all parameters responsible for controlling all envelope parameters
    */
    void linkEnvParameter(std::atomic<float>* ptrToAttack, std::atomic<float>* ptrToDecay, std::atomic<float>* ptrToSustain, std::atomic<float>* ptrToRelease,
        std::atomic<float>* ptrAttackFilter, std::atomic<float>* ptrDecayFilter, std::atomic<float>* ptrSustainFilter, std::atomic<float>* ptrReleaseFilter
        ,std::atomic<float>* ptrToGlobalAttack, std::atomic<float>* ptrToGlobalDecay, std::atomic<float>* ptrToGlobalSustain, std::atomic<float>* ptrToGlobalRelease)
    {
        // Gain envelope parameters
        attackParam = ptrToAttack;
        decayParam = ptrToDecay;
        releaseParam = ptrToRelease;
        sustainParam = ptrToSustain;

        // local filter envelope parameters
        attackFilter = ptrAttackFilter;
        decayFilter = ptrDecayFilter;
        sustainFilter = ptrSustainFilter;
        releaseFilter = ptrReleaseFilter;


        // global filter envelope parameters
        globalAttack = ptrToGlobalAttack;
        globalDecay = ptrToGlobalDecay;
        globalSustain = ptrToGlobalSustain;
        globalRelease = ptrToGlobalRelease; 
    }


    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
    Triggers the note off function of envelopes involved
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        env.noteOff();
        clearCurrentNote();
    }
    

    //--------------------------------------------------------------------------
    /**
    The code below does the bulk of the DSP. It calls all the relevant process functions of all classes involved,
    as well as setting key parameter values of all classes involved. It sets the sends the values to the buffer 
    channels.
     */


    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {



        if (env.isActive()) // check to see if this voice should be playing
        {

            // the code below sets appropriate target values for all smoothed values
            smoothGlobalLFOFreq.setTargetValue(globalLFOFrequency);
            smoothGlobalLFOGain.setTargetValue(globalLFOGain);
            smoothGlobalCutoff.setTargetValue(globalCutOff);
            smoothGlobalResonance.setTargetValue(globalResonance);
            smoothFundDetune.setTargetValue(fundamentalDetune);
            smoothGain0.setTargetValue(gain0);
            smoothGain1.setTargetValue(gain1);
            smoothGain2.setTargetValue(gain2);
            smoothGain3.setTargetValue(gain3);
            smoothGain4.setTargetValue(gain4);
            smoothGain5.setTargetValue(gain5);
            smoothGain6.setTargetValue(gain6);
            smoothGain7.setTargetValue(gain7);
            smoothGain8.setTargetValue(gain8);
            smoothLFOFreq.setTargetValue(lfoFreq);
            smoothLFOGain.setTargetValue(lfoGain);
            smoothCutoffFilter.setTargetValue(cutoffFilter);
            smoothResonanceFilter.setTargetValue(resonanceFilter);
            smoothMasterGain.setTargetValue(masterGain);
            smoothLeftGain.setTargetValue(leftChannelGain);
            smoothRightGain.setTargetValue(rightChannelGain);
            smoothTremoloDepth.setTargetValue(tremoloDepth);
            smoothTremoloRate.setTargetValue(tremoloRate);
            smoothTremoloPulseWidth.setTargetValue(tremoloPulseWidth);


         
            

            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {

                // the code below calculates the next value for all the smoothed parameters
                float globalLFOFreqVal = smoothGlobalLFOFreq.getNextValue();            
                float globalLFOGainVal = smoothLFOGain.getNextValue();
                float globalCutoffVal = smoothGlobalCutoff.getNextValue();
                float globalResonanceVal = smoothGlobalResonance.getNextValue();
                float fundamentalDetuneVal = smoothFundDetune.getNextValue();
                float gain0Val = smoothGain0.getNextValue();
                float gain1Val = smoothGain1.getNextValue();
                float gain2Val = smoothGain2.getNextValue();
                float gain3Val = smoothGain3.getNextValue();
                float gain4Val = smoothGain4.getNextValue();
                float gain5Val = smoothGain5.getNextValue();
                float gain6Val = smoothGain6.getNextValue();
                float gain7Val = smoothGain7.getNextValue();
                float gain8Val = smoothGain8.getNextValue();
                float lfoFreqVal = smoothLFOFreq.getNextValue();
                float lfoGainVal = smoothLFOGain.getNextValue();
                float cutoffFilterVal = smoothCutoffFilter.getNextValue();
                float resonanceFilterVal = smoothResonanceFilter.getNextValue();
                float masterGainVal = smoothMasterGain.getNextValue();
                float leftChannelGainVal = smoothLeftGain.getNextValue();
                float rightChannelGainVal = smoothRightGain.getNextValue();
                float tremoloDepthVal = smoothTremoloDepth.getNextValue();
                float tremoloRateVal = smoothTremoloRate.getNextValue();
                float tremoloPulseWidthVal = smoothTremoloPulseWidth.getNextValue();


                globalFilterLFO.setFrequency(globalLFOFreqVal);                                                             // sets LFO frequency determined by appropriate parameter
                float lfoFilter = globalFilterLFO.process(1, 0) * globalLFOGainVal;                                         // calculates the output value of the LFO

                globalFilter.setFilterType(globalFilterType);                                                               // sets the global filter type

                float cutoff = globalCutoffVal + lfoFilter;                                                                 // calculates the cutoff including the LFO value

                // the code below ensures the cutoff frequency is in an appropriate range
                if (cutoff <= 0.0f)
                    cutoff = 1.0f;
                else if (cutoff > 22000.0f)
                    cutoff = 20000.0f;

                globalFilter.setFrequencyEnvelope(cutoff);                                                                  // sets the cutoff frequency to the appropriate value
                globalFilter.setResonance(globalResonanceVal);                                                              // sets the filter resonance


                float envVal = env.getNextSample();                                                                         // calculates the next gain envelope value 






                outputSound.setGains(gain0Val, gain1Val, gain2Val, gain3Val, gain4Val, gain5Val, gain6Val, gain7Val, gain8Val);                    // sets the gains of the drawbars determined by parameters
                float sample = outputSound.process(waveform, pulseWidth);                                                   // calculates unfiltered output sample


                lfo.setFrequency(lfoFreqVal);                                                                               // sets frequency of an lfo
                float filterFluctuate = (lfoGainVal * lfo.process(1,0));                                                    // calculates lfo frequency current sample
                filter.setFilterType(filterType);                                                                           // determines filter type 
                filter.setFrequencyEnvelope(cutoffFilterVal * freq + (freq * filterFluctuate*cutoffFilterVal)+0.001f);      // determines cutoff frequency
                filter.setResonance(resonanceFilterVal);                                                                    // determines filter resonance



                //delay.setBufferSize(SR);
                //delay.setFeedback(0.1);
                //delay.setDelayTime(SR * 0.5f);


                tremolo.setFrequency(tremoloRateVal);                                                                       // sets tremolo rate
                float tremoloGain = (tremolo.process(1, tremoloPulseWidthVal) * 0.5f  - 1) * tremoloDepthVal;                           // calculates tremolo depth

                float finalGain = (masterGainVal + (tremoloGain * masterGainVal)) * envVal;                                 // includes all gain parameters


                float currentSample = finalGain * globalFilter.process(filter.process(sample));                             // filters the output sample and multiplies it by the envelope value
                //float delaySample =  delay.process(currentSample);
                
                panningLFO.setFrequency(panningFreq);
                float panLeft = (panningLFO.process(1, 0.5) * 0.5f + 0.5f) * leftChannelGain;
                float panRight = (panningLFO.process(1, 0.5) * 0.5f + 0.5f) * rightChannelGain;
                // for each channel, write the currentSample float to the output
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                outputBuffer.addSample(0, sampleIndex, panLeft * (0.2f * currentSample));                       // sends the value to the left channel, multiplying by appropriate channel gain
                outputBuffer.addSample(1, sampleIndex, panRight * (0.2f * currentSample));                      // sends the value to the right channel, multiplying by appropriate channel gain
                }
        }

    }

    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<MySynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------

    int SR;                                                 // stores the sample rate of the plug in

    float masterGain;                                       // controls the overall gain of the sound

    juce::SmoothedValue<float> smoothMasterGain;            // initialises a smoothed value to ensure clickless transitions
    
    int waveform;                                           // variable to control the waveform type
    float pulseWidth;                                       // variable to control the pulse width - only important when the waveform is square

    juce::SmoothedValue<float>  smoothPulseWidth;           // initialises a smoothed value to ensure clickless transitions

    float freq;                                             // variable that holds the frequency of the triggered note

    int tonality;                                           // variable to determine which third is included as a drawbar (major or minor)


    Drawbars outputSound;                                   // creates an instance of the drawbars class

    // the below creates an instance of the juce ADSR envelope class and
    // creates variables that will be used to control the parameters

    juce::ADSR env;
    std::atomic<float>* attackParam;
    std::atomic<float>* decayParam;
    std::atomic<float>* sustainParam;
    std::atomic<float>* releaseParam;


    //==============================================================

    Filter filter;                                          // creates an instance of the filter class

    int filterType;                                         // a variable that will store the filter type (lowpass, highpass, bandpass)
    float cutoffFilter;                                     // a variable to store the cutoff frequency of the filter
    float resonanceFilter;                                  // a variable to store the resonance of the filter

    juce::SmoothedValue<float> smoothCutoffFilter;          // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothResonanceFilter;       // initialises a smoothed value to ensure clickless transitions

    Phasor lfo;                                             // creates an instance of the phasor class that will act as an LFO changing the cutoff Frequency
    float lfoFreq;                                          // a variable that will determine the frequency of the LFO
    float lfoGain;                                          // a variable that will determine the gain of the LFO

    juce::SmoothedValue<float> smoothLFOFreq;               // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothLFOGain;               // initialises a smoothed value to ensure clickless transitions


    // the variables will control the envelope parameters for the filter above
    std::atomic<float>* attackFilter;
    std::atomic<float>* decayFilter;
    std::atomic<float>* sustainFilter;
    std::atomic<float>* releaseFilter;

    float fundamentalDetune;                                // a parameter that will control the detuning of the drawbars

    juce::SmoothedValue<float> smoothFundDetune;            // initialises a smoothed value to ensure clickless transitions


    //  the variables below control the gain parameters of the drawbars
    float gain0;
    float gain1;
    float gain2;
    float gain3;
    float gain4;
    float gain5;
    float gain6;
    float gain7;
    float gain8;

    juce::SmoothedValue<float> smoothGain0;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain1;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain2;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain3;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain4;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain5;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain6;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain7;                 // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGain8;                 // initialises a smoothed value to ensure clickless transitions


    float leftChannelGain;      //  gain for the left channel
    float rightChannelGain;     // gain for the right channel

    Phasor panningLFO;
    float panningFreq;

    juce::SmoothedValue<float> smoothLeftGain;              // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothRightGain;             // initialises a smoothed value to ensure clickless transitions

    //=====================================================

    Filter globalFilter;                                   // creates another instance of the filter class, this acts on all voices present


    // controls envelope parameters for the global filter

    std::atomic<float>* globalAttack;                       // initialises a smoothed value to ensure clickless transitions
    std::atomic<float>* globalDecay;                        // initialises a smoothed value to ensure clickless transitions
    std::atomic<float>* globalSustain;                      // initialises a smoothed value to ensure clickless transitions
    std::atomic<float>* globalRelease;                      // initialises a smoothed value to ensure clickless transitions

    int globalFilterType;                                   // controls filter type
    float globalCutOff;                                     // controls filter cutoff frequency
    float globalResonance;                                  // controls filter resonance

    juce::SmoothedValue<float> smoothGlobalCutoff;           // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGlobalResonance;        // initialises a smoothed value to ensure clickless transitions



    Phasor globalFilterLFO;                                  // creates an instance of the phasor class to be used as an LFO
    float globalLFOFrequency;                                // creates a variable that will be used to set the LFO frequency
    float globalLFOGain;                                     // controls the LFO gain

    juce::SmoothedValue<float> smoothGlobalLFOFreq;          // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothGlobalLFOGain;          // initialises a smoothed value to ensure clickless transitions



    //============================================


    Phasor tremolo;                                          // creates a phasor that will be a square osc - used for gain control

    float tremoloDepth;
    float tremoloRate;
    float tremoloPulseWidth;

    juce::SmoothedValue<float> smoothTremoloDepth;           // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothTremoloRate;            // initialises a smoothed value to ensure clickless transitions
    juce::SmoothedValue<float> smoothTremoloPulseWidth;      // initialises a smoothed value to ensure clickless transitions

    //Delay delay;

};

