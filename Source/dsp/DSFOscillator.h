#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <algorithm>

class DSFOscillator
{
public:
    explicit DSFOscillator(double sr = 44100.0);

    void setSampleRate(double newSampleRate);
    void setPitch(double newFreq);
    void setFrequency(double newFreq);
    void setModulatedShape(double newShape);
    void setParams(double newA, double newShape);
    
    inline void resetPhase() { phase = 0.0; }
    inline bool isWrapped() const { return wrapped; }

    double processSample(double phaseMod = 0.0);

private:
    void updateEffectiveA();
    void updateNormalization();
    void updateDynamicNormalization();
    
    // Fast, inline soft clipper
    static inline double clip(double x)
    {
        constexpr double threshold = 0.9;
        constexpr double headroom = 0.1;
        
        double absX = std::abs(x);
        if (absX <= threshold) return x; // Highly predictable branch
        
        double e = absX - threshold;
        double val = threshold + (headroom * e) / (headroom + e);
        return (x >= 0.0) ? val : -val;
    }

    // --- HOT VARIABLES (Grouped for CPU cache locality) ---
    double phase = 0.0;
    double inc = 0.0;
    double a = 0.0;
    double a2 = 0.0;          // Precalculated: a * a
    double aN1 = 1.0;         // Precalculated: pow(a, N + 1)
    double N_d = 1.0;         // Precalculated: static_cast<double>(N)
    double offset = juce::MathConstants<double>::pi;
    double amount = 1.0;
    double normGain = 1.0;
    double shapeGain = 1.0;   // Precalculated gain for shape <= 0.5
    bool wrapped = false;

    // --- COLD VARIABLES (Updated only on parameter change) ---
    double sampleRate = 44100.0;
    double freq = 440.0;
    double userA = 0.0;
    double shape = 0.0;
    int N = 1;
    double baseLevel = 1.0;
    double twoPi = juce::MathConstants<double>::twoPi;
};