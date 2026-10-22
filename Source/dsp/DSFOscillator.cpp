#include "DSFOscillator.h"

DSFOscillator::DSFOscillator(double sr)
{
    setSampleRate(sr);
    setParams(0.9, 0.5);
}

void DSFOscillator::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    setFrequency(freq);
}

void DSFOscillator::setPitch(double newFreq)
{
    freq = std::max(10.0, newFreq);
    inc = (twoPi * freq) / sampleRate;
}

void DSFOscillator::setFrequency(double newFreq)
{
    setPitch(newFreq);

    double maxFreq = std::min(20000.0, sampleRate * 0.45);
    double topHarmonic = std::floor(maxFreq / freq);
    int newN = std::max(1, static_cast<int>(topHarmonic - 1.0));
    
    if (newN != N)
    {
        N = newN;
        N_d = static_cast<double>(N); // Update precalculated double
        updateEffectiveA();
        updateNormalization();
    }
}

void DSFOscillator::setModulatedShape(double newShape)
{
    shape = std::clamp(newShape, 0.0, 1.0);

    if (shape <= 0.5)
    {
        offset = juce::MathConstants<double>::pi * (shape * 2.0);
        amount = 1.0;
    }
    else
    {
        offset = juce::MathConstants<double>::pi;
        amount = 1.0 - ((shape - 0.5) * 2.0);
    }

    offset = std::max(offset, 0.05);
    
    // Precalculate the gain to remove the branch from the sample loop
    shapeGain = (shape <= 0.5) ? std::min(juce::MathConstants<double>::pi / offset, 3.0) : 1.0;
    
    updateDynamicNormalization();
}

void DSFOscillator::setParams(double newA, double newShape)
{
    userA = std::clamp(newA, 0.01, 0.999);
    shape = std::clamp(newShape, 0.0, 1.0);

    if (shape <= 0.5)
    {
        offset = juce::MathConstants<double>::pi * (shape * 2.0);
        amount = 1.0;
    }
    else
    {
        offset = juce::MathConstants<double>::pi;
        amount = 1.0 - ((shape - 0.5) * 2.0);
    }

    offset = std::max(offset, 0.05);
    shapeGain = (shape <= 0.5) ? std::min(juce::MathConstants<double>::pi / offset, 3.0) : 1.0;
    
    updateEffectiveA();
    updateNormalization();
}

void DSFOscillator::updateEffectiveA()
{
    constexpr double N_ref = 64.0;
    if (N_d < N_ref)
    {
        double scaleFactor = std::pow(N_d / N_ref, 0.15);
        a = std::pow(userA, 1.0 / std::max(0.2, scaleFactor));
    }
    else
    {
        a = userA;
    }
    
    a2 = a * a;                  // Precalculate for hot loop
    aN1 = std::pow(a, N_d + 1.0); // Precalculate for hot loop
}

void DSFOscillator::updateNormalization()
{
    double theta = std::atan2(1.0 - a2, 2.0 * a);
    double cosT = (2.0 * a) / (1.0 + a2);
    double sinT = (1.0 - a2) / (1.0 + a2);

    double sinN = std::sin(N_d * theta);
    double sinNP1 = std::sin((N_d + 1.0) * theta);

    double num = a * sinT + aN1 * (a * sinN - sinNP1);
    double den = 1.0 + a2 - 2.0 * a * cosT;
    double peakN = std::abs(num / std::max(den, 1e-7));

    double a2N = std::pow(a2, N_d);
    double rmsN = a * std::sqrt((1.0 - a2N) / std::max(1.0 - a2, 1e-7));
    baseLevel = std::sqrt(peakN * rmsN);

    updateDynamicNormalization();
}

void DSFOscillator::updateDynamicNormalization()
{
    double sineBlend = std::sqrt(1.0 + amount * amount - 2.0 * amount * std::cos(offset));
    double pulseBlend = std::max(1.0, amount);
    double blendFactor = pulseBlend + (sineBlend - pulseBlend) * (1.0 - a);

    // Use precalculated shapeGain instead of recalculating pulseGain
    double totalLevel = baseLevel * blendFactor * shapeGain;
    normGain = 0.8 / std::max(totalLevel, 1e-7);
}

double DSFOscillator::processSample(double phaseMod)
{
    double theta = phase + phaseMod;
    double thetaOff = theta + offset;

    // --- R calculation ---
    double sin1 = std::sin(theta);
    double cos1 = std::cos(theta);
    double sinN = std::sin(N_d * theta);
    double cosN = std::cos(N_d * theta);

    double sinNP1 = sinN * cos1 + cosN * sin1;
    double numR = a * sin1 + aN1 * (a * sinN - sinNP1);
    double denR = 1.0 + a2 - 2.0 * a * cos1;
    denR = (denR < 1e-12) ? 1e-12 : denR; // Branchless max
    double r = numR / denR;

    // --- Q calculation ---
    double sinOff = std::sin(thetaOff);
    double cosOff = std::cos(thetaOff);
    double sinNOff = std::sin(N_d * thetaOff);
    double cosNOff = std::cos(N_d * thetaOff);

    double sinNP1Off = sinNOff * cosOff + cosNOff * sinOff;
    double numQ = a * sinOff + aN1 * (a * sinNOff - sinNP1Off);
    double denQ = 1.0 + a2 - 2.0 * a * cosOff;
    denQ = (denQ < 1e-12) ? 1e-12 : denQ; // Branchless max
    double q = numQ / denQ;

    // All gains applied in one multiplication, no branches
    double output = (r - q * amount) * shapeGain * normGain;
    
    phase += inc;
    wrapped = false;
    if (phase >= twoPi)
    {
        phase -= twoPi;
        wrapped = true;
    }

    return clip(output * 0.25);
}