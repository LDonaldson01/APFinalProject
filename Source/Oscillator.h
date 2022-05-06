#ifndef Oscillators_h
#define Oscillators_h
#include <cmath>

/**
Oscillator class with three wave shapes (processSine, processSquare, processTriangle)
 */
class Phasor
{
public:
    float getIncrement()
    {
        return phaseDelta;
    }

    float getLastOutput()
    {
        return lastOutput;
    }

    double poly_blep(double t)
    {
        // 0 <= t < 1
        if (t < phaseDelta)
        {
            t /= phaseDelta;
            // 2 * (t - t^2/2 - 0.5)
            return t + t - t * t - 1.;
        }

        // -1 < t < 0
        else if (t > 1. - phaseDelta)
        {
            t = (t - 1.) / phaseDelta;
            // 2 * (t^2/2 + t + 0.5)
            return t * t + t + t + 1.;
        }

        // 0 otherwise
        else
        {
            return 0;
        }
    }

    virtual float output(float p) {

        return poly_blep(p);
    }

    float process(int type, float pulseWidth)
    {
        phase += phaseDelta;

        if (phase > 1.0f)
            phase -= 1.0f;

        pulseWidth = fmod(pulseWidth, 1);

        float value;
        if (type == 0)
            value =poly_blep((2.0 * phase / 3.141593) - 1.0);
        else if (type == 1)
            value = sin(phase * 2.0 * 3.141593);
        else if (type == 2)
        {
            value = -1.0f + phase / 3.141593;
            value = 2.0 * (fabs(value) - 0.5);
            value = phaseDelta * value + (1 - phaseDelta) * lastOutput;
            lastOutput = value;
        }
        else  if (type == 3)
        {
            value = 0.5;
            if (phase > pulseWidth)
                value = -0.5;

            value += poly_blep(phase);
            value -= poly_blep(fmod(phase + 0.5, 1.0));
        }
        return value;
    }

    /**
     set the sample rate - needs to be called before setting frequency or using process

     @param SR samplerate in Hz
     */
    void setSampleRate(float SR)
    {
        sampleRate = SR;
    }

    /**
     set the oscillator frequency - MAKE SURE YOU HAVE SET THE SAMPLE RATE FIRST

     @param freq oscillator frequency in Hz
     */
    void setFrequency(float freq)
    {
        frequency = freq;
        phaseDelta = frequency / sampleRate;
    }

    float getSampleRate()
        {
            return sampleRate;
        }
 
private:
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float lastOutput = 0.0f;
    float phaseDelta;
};
//==========================================

class TriOsc : public Phasor {


    float output(float p) override {
        float value = fabsf(p - 0.5f) - 0.5f;
        value = getIncrement() * value + (1 - getIncrement()) * getLastOutput();
    }

};

//==========================================



#endif /* Oscillators_h */
