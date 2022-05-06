#pragma once
#include <JuceHeader.h>
#include <iostream>


/**
*	This class uses the juce::IIRFilter class. The user must set the plug-in sample rate, and then 
* allows the user to input the desired filter type, cut-off frequency and filter resonance. The class
* process will then output a filtered input sample.
*/
class Filter
{
public:

/**
* sets the sample rate of the envelope and stores it
*/
	void setSampleRate(int SR)
	{
		sampleRate = SR;
		env.setSampleRate(SR);
	}

/**
* takes an integer as an input that corresponds to a filter type (lowpass, highpass, bandpass)
*/
	void setFilterType(int type)
	{
		filterType = type;
		if (filterType == 0)
			filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, resonance));
		else if (filterType == 1)
			filter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, cutoff, resonance));
		else if (filterType == 2)
			filter.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, cutoff, resonance));
	}


	/**
	* sets the cutoff frequency of the filter. This function is to be used if there is an associated filter
	* envelope with defined parameters.
	*/
	void setFrequencyEnvelope(float input)
	{
		float envVal = env.getNextSample();
		cutoff = envVal * input;

		if (cutoff <= 0.0f)
			cutoff = 0.1f;
		else if (cutoff > 24000.0f)
			cutoff = 20000.0f;

	}

	/**
	* This function is to be used when there is no filter envelope
	*/
	void setFrequency(float input)
	{
		cutoff = input;
	}


	/**
	* This function takes in pointers to parameter values that will control the envelope values of the filter
	*/
	void setEnvelope(std::atomic<float>* attack, std::atomic<float>* decay, std::atomic<float>* sustain, std::atomic<float>* release)
	{

		// sets envelope parameters
		juce::ADSR::Parameters envParams;
		envParams.attack = *attack;
		envParams.decay = *decay;
		envParams.sustain = *sustain;
		envParams.release = *release;

		env.setParameters(envParams);

		env.reset();
		env.noteOn();
	}


	/**
	* sets the filter resonance
	*/
	void setResonance(float reso)
	{
		resonance = reso;
	}


	/**
	* takes in an input sample and returns a filtered output signal
	*/
	float process(float sample)
	{
		float output;
		if (filterType >= 3)
			output = sample;
		else
		   output = filter.processSingleSampleRaw(sample);
		return output;
	}

private:
	int sampleRate;					// variable to store sampleRate
	float cutoff = 0.1f;			// variable to store the cutoff frequency
	juce::IIRFilter filter;			// initialises an instance of the juce filter class
	int filterType;					// variable to store the current filter type
	float resonance = 0.001f;		// variable to store filter resonance


	
	juce::ADSR env;					// initialises the filter envelope
};
