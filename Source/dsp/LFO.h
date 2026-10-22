#pragma once
#include <cmath>
#include <algorithm>
#include <juce_core/juce_core.h>

class LFO
{
public:
    LFO() = default;
    ~LFO() = default;

    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateIncrement();
    }

    void setRate(double rateHz)
    {
        currentRate = std::max(0.01, rateHz);
        updateIncrement();
    }

    void setShape(double shapeNorm)
    {
        // 0.0 = Pure Sine, 1.0 = Pure Triangle (or blend them smoothly)
        shape = std::clamp(shapeNorm, 0.0, 1.0);
    }

    double processSample()
    {
        phase += phaseIncrement;
        if (phase >= 1.0)
            phase -= 1.0;

        // 1. Sine Wave computation
        double sineVal = std::sin(phase * 2.0 * juce::MathConstants<double>::pi);

        // 2. Triangle Wave computation (ramp up, ramp down, symmetrical)
        double triVal = 0.0;
        if (phase < 0.25)
        {
            triVal = 4.0 * phase;
        }
        else if (phase < 0.75)
        {
            triVal = 2.0 - (4.0 * phase);
        }
        else
        {
            triVal = (4.0 * phase) - 4.0;
        }

        // Crossfade / morph between sine and triangle based on shape parameter
        return (sineVal * (1.0 - shape)) + (triVal * shape);
    }

    void reset()
    {
        phase = 0.0;
    }

private:
    void updateIncrement()
    {
        if (sampleRate > 0.0)
            phaseIncrement = currentRate / sampleRate;
    }

    double sampleRate = 44100.0;
    double currentRate = 1.0;
    double shape = 0.0;
    double phase = 0.0;
    double phaseIncrement = 0.0;
};