// BeatFrequency.h.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "Oscillator.h"


/**
* This class acts similarly to drawbars on a hammond organ. It will take an input oscillator, and allows you to add 
* other oscillators at set harmonics to the output sample. You must set the gains of each corresponding harmonic.
*/

	//=======================================================================================

class Drawbars
{

public:

	/**
	*	This function takes in a 0 or 1 and sets the corresponding tonality of the available harmonics,
	*	0 corresponds to minor and 1 to major
	*/
	void setFrequencyRatios(int tonality)
	{
		frequencyRatio[0] = 0.25f;
		

		if (tonality == 0)
		{
			frequencyRatio[1] = 1.189f * 0.5f;
			frequencyRatio[6] = 1.189f * 2.0f;
		}
		else
		{
			frequencyRatio[1] = 1.26f * 0.5f;
			frequencyRatio[6] = 1.26f * 2.0f;
		}

		frequencyRatio[2] = 0.5f;
		frequencyRatio[3] = 1.0f;
		frequencyRatio[4] = 2.996f * 0.5f;
		frequencyRatio[5] = 2.0f;
		
		frequencyRatio[7] = 1.498f * 2.0f;
		frequencyRatio[8] = 2.5f;
	}

	/**
	* This function sets the gain of each harmonic
	*/
	void setGains(float gain0, float gain1, float gain2, float gain3, float gain4, float gain5, float gain6, float gain7, float gain8)
	{
		gains[0] = gain0;
		gains[1] = gain1;
		gains[2] = gain2;
		gains[3] = gain3;
		gains[4] = gain4;
		gains[5] = gain5;
		gains[6] = gain6;
		gains[7] = gain7;
		gains[8] = gain8;
	}


	/**
	* This function takes in the sample rate and fills a vector with oscillators, and sets the sample rate
	*/
	void fillOscillatorVector(float SR)
	{
		for (int i = 0; i < 9; i++)
		{
			oscillators.push_back(Phasor());
			oscillators[i].setSampleRate(SR);
			sampleRate = SR;


			detuneOscillators.push_back(Phasor());
			detuneOscillators[i].setSampleRate(SR);
		}
	}

	/**
	*	Sets the oscillators in the vector to approriate frequencies
	*/
	void setFrequencies(float freq, float detune)
	{
		fundamental = freq;
		for (int i = 0; i < 9; i++)
		{
			float frequency = fundamental * frequencyRatio[i];			// calculates relevant frequency
			float detuneFrequency = frequency * detune + frequency;		// calculates the frequency of the detuned oscillator (detune is a percentage)
			


			// checks to ensure frequency is below the Nyquist
			if (0.5f * frequency < sampleRate / 2.0f)
				oscillators[i].setFrequency(frequency * 0.5f);		// sets frequency an octave below the triggered note
			else
				oscillators[i].setFrequency(0.1f);					// sets the frequency to an inaudible frequency is it would otherwise cause aliasing

						// checks to ensure frequency is below the Nyquist
			if (0.5f * detuneFrequency < sampleRate / 2.0f)
				detuneOscillators[i].setFrequency(detuneFrequency * 0.5f);	// sets frequency an octave below the triggered note
			else
				detuneOscillators[i].setFrequency(0.1f);					// sets the frequency to an inaudible frequency is it would otherwise cause aliasing
		}

	}


	/**
	*	This function takes in a waveform parameter, pulse width value and a detune value and then adds the
	*	harmonics to give an output sound
	*/
	float process(int waveform, float pulseWidth)
	{
		float sample = 0.0f;
		for (int i = 0; i < 9; i++)
		{
			sample += gains[i] * (oscillators[i].process(waveform, pulseWidth) + detuneOscillators[i].process(waveform,pulseWidth));	// multiplies each oscillator by the appropriate gain

		}


		return 0.5f * sample;
	}

private:

	std::vector<Phasor> oscillators;		// vector holding the oscillators
	std::vector<Phasor> detuneOscillators;	// vector holding the detuned oscillators

	float gains[9];							// initialises static array to hold gain values
	float frequencyRatio[9];				// initialises static array to hold frequency ratio values

	float fundamental;						// stores fundamental frequency
	float sampleRate;						// stores sample rate

	
};


