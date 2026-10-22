#include "ZDFfilter.h"
#include <cmath>
#include <algorithm>
#include "DSPUtils.h"

ZDFfilter::ZDFfilter()
    : s1(0.0), s2(0.0), s3(0.0)
{
}

void ZDFfilter::reset()
{
    s1 = 0.0;
    s2 = 0.0;
    s3 = 0.0;
}

double ZDFfilter::softSatC2_deriv(double x)
{
    double z = x / 1.875;
    if (z <= -1.0 || z >= 1.0) return 0.0;
    double omz2 = 1.0 - z * z;
    return omz2 * omz2;
}

double ZDFfilter::process(double inSample, double cutoffHz, double resonance, double sampleRate, double morph)
{
    // 1. Pre-warp cutoff frequency and calculate TPT gain
    double cutoff = std::min(cutoffHz, sampleRate * 0.441);
    double g = std::tan((juce::MathConstants<double>::pi * cutoff) / sampleRate);
    double G = g / (1.0 + g);

    double G2 = G * G;
    double G3 = G2 * G;

    double k = std::max(0.0, resonance);

    // Resonance-dependent input compensation (restores low-end as k rises)
    // When k == 0 → driveComp == 1.0 (no change)
    double driveComp = 1.0 + 0.5 * k;          // try 0.4 … 0.8 to taste
    double compensatedIn = inSample * driveComp;

    // 2. Exact TPT state-feedback vector for 3 stages
    double S = (1.0 - G) * (G2 * s1 + G * s2 + s3);

    // 3. Initial linear guess for zero-delay node 'u'
    double u = (compensatedIn - k * S) / (1.0 + k * G3);

    // 4. Three Newton-Raphson iterations
    for (int i = 0; i < 3; ++i)
    {
        double feedbackNode = G3 * u + S;

        double f = JolleyDSP::softClip(feedbackNode);
        double df = softSatC2_deriv(feedbackNode);

        double residual = u + k * f - compensatedIn;
        double derivative = 1.0 + k * G3 * df;

        u -= residual / derivative;
    }

    // 5. Three cascaded TPT integrators
    double v1 = (u - s1) * G;
    double y1 = v1 + s1;
    s1 = y1 + v1;

    double v2 = (y1 - s2) * G;
    double y2 = v2 + s2;
    s2 = y2 + v2;

    double v3 = (y2 - s3) * G;
    double y3 = v3 + s3;
    s3 = y3 + v3;

    // 6. Derive 18 dB/octave Mode Taps with proper unity-gain scaling
    double lp18 = y3;
    double bp18 = 3.0 * (y1 - 2.0 * y2 + y3);
    double hp18 = u - 3.0 * y1 + 3.0 * y2 - y3;

    // 7. Continuous SVF Morph (LP → BP → HP)
    double m = std::clamp(morph, 0.0, 2.0);

    if (m <= 1.0)
    {
        // Clean crossfade from pure Lowpass (y3) to Bandpass
        return (1.0 - m) * lp18 + m * bp18;
    }
    else
    {
        // Crossfade from Bandpass to Highpass
        double t = m - 1.0;
        return (1.0 - t) * bp18 + t * hp18;
    }
}