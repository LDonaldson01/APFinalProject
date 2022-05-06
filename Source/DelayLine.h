// DelayLine.h.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cmath>


/**
* Implements a delay
*/
class Delay
{
public:


	/**
	*	sets the maximum delay length
	*/
	void setBufferSize(int sizeInSamples)
	{
		bufferSize = sizeInSamples;
		buffer = new float[bufferSize];			// initialises buffer array
		for (int i = 0; i < bufferSize; i++)
		{
			buffer[i] = 0.0f;					// sets all values to zero
		}
	}


	/**
	* sets a feedback value
	*/
	void setFeedback(float gain)
	{

		// ensures feedback value is in range 0-1
		if (gain > 1)
			feedback = 1;
		else if (gain < 0)
			feedback = 0;
		else
			feedback = gain;

	}

	/**
	*	will write current sample into the buffer array
	*/
	void writeSample(float input)
	{
		buffer[writePosition] = input;
		writePosition += 1;

		// ensures write position is in range 0-maximum delay time
		if (writePosition >= bufferSize)
			writePosition = 0;
	}


	/**
	*	The function below will linearly interpolate between the two nearest values of the readPosition
	*/
	float linearInterpolation()
	{
		int A = floor(readPosition);
		int B = A + 1;

		if (B >= bufferSize)
			B -= bufferSize;

		float a = buffer[A];
		float b = buffer[B];


		return (readPosition - A) * (b - a) + a;
	}

	/**
	* this value returns the value of the read pointer. For non-integer indexing it uses linear interpolation.
	*/
	float readSample()
	{
		float value = linearInterpolation();

		readPosition++;

		// ensures the read position is in the range of the buffer array
		if (readPosition >= bufferSize)
		{
			readPosition = 0;
		}
		return value;
	}



	/**
	* This function will read the relevant delay sample, write in the current sample and return the delayed sample
	*/
	float process(float input)
	{
		
		float value = readSample();
		writeSample(input + feedback * value);

		return value;
	}


	/**
	* This function sets the delay time in samples
	*/
	void setDelayTime(float time)
	{
		delayInSamples = time;

		// ensures the delay time is within appropriate range
		if (delayInSamples < 1)
			delayInSamples = 1;
		if (delayInSamples > bufferSize)
			delayInSamples = bufferSize;

		// sets the read position the appropriate number of samples behind the write position
		readPosition = writePosition - delayInSamples;
		if (readPosition < 0)
			readPosition += bufferSize;
	}

	~Delay()
	{
		delete[] buffer;
	}


private:
	float sampleRate;			// holds the sample rate
	int bufferSize;				// holds the size of the buffer array
	float* buffer;				// initialises a pointer to a buffer array				
	float readPosition= 0;		// initialises a read pointer
	int writePosition = 0;		// initialises a write pointer	
	float delayInSamples;		// initialises variable to store delay time
	float feedback;				// initialises afeedback control

};