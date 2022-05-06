#pragma once
#include <JuceHeader.h>


/**
* This class when called will output a randomly generated sequence of notes from a scale. The
* class requires you to choose a scale from those coded in, and requires you to set a tempo in bpm and 
* how many notes per bar there will be. 
*/

class Sequencer 
{
public:

	/**
	* This function sets the sample rate for all the oscillators used in the class
	*/
	void setSampleRate(int SR)
	{
		sampleRate = SR;

		for (int i = 0; i < oscCount; ++i)
		{
			oscillators.push_back(Phasor());
			oscillators[i].setSampleRate(SR);
		}
	}

	/**
	* This function sets the tempo, in bpm. The number of seconds per beat is then calculated from this.
	*/
	void setTempo(float bpm)
	{
		tempo = bpm;

		secondsPerBeat = (1.0f / tempo) * 60.0f;		// calculates seconds per beat

		timer = secondsPerBeat / float(notesPerBeat);
	}


	/**
	* This function takes an integer as an input which represents a certain scale. It will then fill a static-array
	* with values that when multiplied by the frequency of a root note will give the frequency a certain note of the scale
	*/
	void fillRatio(int scale)
	{
		frequencyRatio[0] = 1.0f;			// root note
		frequencyRatio[7] = 2.0f;			// octave up
		frequencyRatio[8] = 0.5f;			// octave down

		// harmonic major
		if (scale == 1)
		{
			frequencyRatio[1] = 1.122f;	
			frequencyRatio[2] = 1.26f;
			frequencyRatio[3] = 1.335f;
			frequencyRatio[4] = 1.498f;
			frequencyRatio[5] = 1.682f;
			frequencyRatio[6] = 1.888f;
			
		}
		//harmonic minor
		else if (scale == 0)
		{
			frequencyRatio[1] = 1.122f;
			frequencyRatio[2] = 1.189;
			frequencyRatio[3] = 1.335f;
			frequencyRatio[4] = 1.498f;
			frequencyRatio[5] = 1.587;
			frequencyRatio[6] = 1.782f;

		}
		// mixolydian mode
		else if (scale == 2)
		{
			frequencyRatio[1] = 1.122f;
			frequencyRatio[2] = 1.26f;
			frequencyRatio[3] = 1.335f;
			frequencyRatio[4] = 1.498f;
			frequencyRatio[5] = 1.682f;
			frequencyRatio[6] = 1.782f;
		}

		// phyrgian mode
		else if (scale == 3)
		{
			frequencyRatio[1] = 1.059f;
			frequencyRatio[2] = 1.189f;
			frequencyRatio[3] = 1.335f;
			frequencyRatio[4] = 1.498f;
			frequencyRatio[5] = 1.587f;
			frequencyRatio[6] = 1.782f;
		}

		// locrian mode
		else if (scale == 4)
		{
			frequencyRatio[1] = 1.059f;
			frequencyRatio[2] = 1.189f;
			frequencyRatio[3] = 1.335f;
			frequencyRatio[4] = 1.414f;
			frequencyRatio[5] = 1.587f;
			frequencyRatio[6] = 1.782f;
		}
	}

	/**
	* This function sets the frequency of the oscillator that will be played, as well as the detuned oscillator
	*/
	void setFreq(float input)
	{
		root = input;	// sets the root note
		auto randomInt = juce::Random::getSystemRandom().nextInt(8);	// generates a random variable in the range 0-8
		freq = root * frequencyRatio[randomInt];						// multiplies the root note by a random element in the ratio list
		
																		
		// the code below sets the output oscillators frequency to the calculated frequency above, as well
		// as setting the frequency for the harmonious oscillator and detuned oscillator for both of these

		oscillators[0].setFrequency(freq);
		oscillators[1].setFrequency(freq + freq * detuneParam);
		oscillators[3].setFrequency(freq * 1.498f);
		oscillators[4].setFrequency(1.498f * (freq + freq * detuneParam));
	}

	void setNotesPerBeat(int param)
	{
		notesPerBeat = param;
	}

	/**
	* this function takes in as parameters the waveform type of the oscillator, the sample count,
	* the percentage of detuning to add and a binary deciding to add an a harmony or not
	*/
	float process(int waveform, int count, float detune, int harmony)
	{
		detuneParam = detune;	// sets the detune amount

		// checks to see if we have passed a given number of samples (controlled by the bpm and notes per beat)
		if (count % int(timer * sampleRate) == 0)
		{
			setFreq(root);		// calls the frequency function, giving a randomly generated frequency
		}

		float sample = 0.0f;


		// adds the oscillators with appropriate frequencies to an output sample

		if(harmony == 0)
			sample = (oscillators[0].process(waveform, 0.5) + oscillators[1].process(waveform, 0.5));
		else
		{
			
			for (int i = 0; i < oscCount; i++)
			{
				sample += oscillators[i].process(waveform, 0.5);
			}
		}
		return sample;
	}


private:

	float freq;							// variable to hold the output oscillator frequency 
	float root;							// variable to hold the root frequency


	int sampleRate;						// variable to hold the sample rate

	float timer;						// variable to store how long a given note should be played for

	float tempo;						// variable to hold the bpm

	float secondsPerBeat;				// variable to hold the number of seconds per beat
	int notesPerBeat;					// variable to hold the numbrr of notes per beat

	float frequencyRatio[9];			// initialises an array to hold the ratios of frequencies that can be called


	float detuneParam = 0.01f;			// initialises a parameter that will store the detune parameter

	std::vector<Phasor> oscillators;	// intialises a vector of oscillators that are used for the final output
	int oscCount = 4;					// sets the number of oscillators
};