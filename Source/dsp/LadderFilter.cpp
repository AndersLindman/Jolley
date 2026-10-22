#include "LadderFilter.h"
#include <cmath>
#include <algorithm>
#include "DSPUtils.h"

LadderFilter::LadderFilter()
    : s1(0.0), s2(0.0), s3(0.0), s4(0.0)
{
}

void LadderFilter::reset()
{
    s1 = 0.0;
    s2 = 0.0;
    s3 = 0.0;
    s4 = 0.0;
}

double LadderFilter::softSatC2_deriv(double x)
{
    double z = x / 1.875;
    if (z <= -1.0 || z >= 1.0) return 0.0;
    double omz2 = 1.0 - z * z;
    return omz2 * omz2;
}

double LadderFilter::process(double inSample, double cutoffHz, double resonance, double sampleRate, double morph, bool use24dB)
{
    double cutoff = std::min(cutoffHz, sampleRate * 0.49);
    double g = std::tan((juce::MathConstants<double>::pi * cutoff) / sampleRate);
    double G = g / (1.0 + g);

    double G2 = G * G;
    double G3 = G2 * G;
    double G4 = G3 * G;

    double k = resonance;
    constexpr double compensationAmount = 0.5;
    double inputComp = inSample * (1.0 + k * compensationAmount);

    // State feedback vector
    double S = (1.0 - G) * (G3 * s1 + G2 * s2 + G * s3 + s4);

    // Solve feedback node 'u'
    double u = (inputComp - k * S) / (1.0 + k * G4);

    for (int i = 0; i < 3; ++i)
    {
        double feedbackNode = G4 * u + S;
        double f = JolleyDSP::softClip(feedbackNode);
        double df = softSatC2_deriv(feedbackNode);

        double residual = u + k * f - inputComp;
        double derivative = 1.0 + k * G4 * df;

        u -= residual / derivative;
    }

    // TPT Integrator Cascade
    double v1 = (u - s1) * G;
    double y1 = v1 + s1;
    s1 = y1 + v1;

    double v2 = (y1 - s2) * G;
    double y2 = v2 + s2;
    s2 = y2 + v2;

    double v3 = (y2 - s3) * G;
    double y3 = v3 + s3;
    s3 = y3 + v3;

    double v4 = (y3 - s4) * G;
    double y4 = v4 + s4;
    s4 = y4 + v4;

    // --- Derive Pure Mode Taps ---
    double lp = 0.0, bp = 0.0, hp = 0.0;

    if (use24dB)
    {
        // 24dB / octave modes
        lp = y4;
        bp = 4.0 * (y2 - 2.0 * y3 + y4);
        hp = u - 4.0 * y1 + 6.0 * y2 - 4.0 * y3 + y4;
    }
    else
    {
        // 12dB / octave modes
        lp = y2;
        bp = 2.0 * (y1 - y2);
        hp = u - 2.0 * y1 + y2;
    }

    // --- SVF Morphing (LP -> BP -> HP) ---
    double m = std::clamp(morph, 0.0, 2.0);

    if (m <= 1.0)
    {
        // Crossfade LP -> BP
        return (1.0 - m) * lp + m * bp;
    }
    else
    {
        // Crossfade BP -> HP
        double t = m - 1.0;
        return (1.0 - t) * bp + t * hp;
    }
}