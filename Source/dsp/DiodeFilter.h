#pragma once
#include <JuceHeader.h>
#include <array>

class DiodeFilter
{
public:
    DiodeFilter();
    ~DiodeFilter() = default;

    void reset();
    double process(double inSample,
                   double cutoffHz,
                   double resonance,
                   double sampleRate,
                   double drive = 2.5,
                   double morph = 0.0,
                   bool use18dB = true);

private:
    double s1;
    double s2;
    double s3;
    double s4;

    // Relative stage capacitances
    std::array<double, 4> capacitanceRatios;
};