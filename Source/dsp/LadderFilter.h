#pragma once
#include <JuceHeader.h>

class LadderFilter
{
public:
    LadderFilter();
    ~LadderFilter() = default;

    void reset();
    double process(double inSample, double cutoffHz, double resonance, double sampleRate, double morph = 0.0, bool use24dB = true);

private:
    double softSatC2(double x);
    double softSatC2_deriv(double x);

    double s1;
    double s2;
    double s3;
    double s4;
};