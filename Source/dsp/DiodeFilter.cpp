#include "DiodeFilter.h"
#include <cmath>
#include <algorithm>

DiodeFilter::DiodeFilter()
    : s1(0.0), s2(0.0), s3(0.0), s4(0.0),
      capacitanceRatios{1.0, 1.0, 0.9, 0.6}
{
}

void DiodeFilter::reset()
{
    s1 = 0.0;
    s2 = 0.0;
    s3 = 0.0;
    s4 = 0.0;
}

double DiodeFilter::process(double inSample,
                            double cutoffHz,
                            double resonance,
                            double sampleRate,
                            double drive,
                            double morph,
                            bool use18dB)
{
    // 1. TB-303-inspired resonance/cutoff interaction.
    double cutoffShift = resonance * 200.0;

    double cutoff = std::min(
        std::max(20.0, cutoffHz - cutoffShift),
        sampleRate * 0.49
    );

    // 2. Calculate an individual TPT coefficient per stage.
    auto getG = [cutoff, sampleRate](double capacitanceRatio) {
        double stageCutoff = std::min(
            cutoff / capacitanceRatio,
            sampleRate * 0.49
        );

        double g = std::tan((juce::MathConstants<double>::pi * stageCutoff) / sampleRate);
        return g / (1.0 + g);
    };

    double G1 = getG(capacitanceRatios[0]);
    double G2 = getG(capacitanceRatios[1]);
    double G3 = getG(capacitanceRatios[2]);
    double G4 = getG(capacitanceRatios[3]);

    // 3. Linear cascade coefficient and state-feedback term.
    double A = G1 * G2 * G3 * G4;

    double S = G2 * G3 * G4 * (1.0 - G1) * s1 +
               G3 * G4 * (1.0 - G2) * s2 +
               G4 * (1.0 - G3) * s3 +
               (1.0 - G4) * s4;

    double k = std::max(0.0, resonance);

    // 4. Initial estimate for the nonlinear feedback solver.
    double u = (inSample - k * S) / (1.0 + k * A);

    // 5. Newton-Raphson iterations with tanh saturation.
    for (int i = 0; i < 3; ++i)
    {
        double feedbackNode = A * u + S;
        double driven = feedbackNode * drive;

        double t = std::tanh(driven);
        double f = t;
        double df = drive * (1.0 - t * t);

        double residual = u + k * f - inSample;
        double derivative = 1.0 + k * A * df;

        u -= residual / derivative;
    }

    // 6. Four cascaded TPT integrators.
    double v1 = (u - s1) * G1;
    double y1 = v1 + s1;
    s1 = y1 + v1;

    double v2 = (y1 - s2) * G2;
    double y2 = v2 + s2;
    s2 = y2 + v2;

    double v3 = (y2 - s3) * G3;
    double y3 = v3 + s3;
    s3 = y3 + v3;

    double v4 = (y3 - s4) * G4;
    double y4 = v4 + s4;
    s4 = y4 + v4;

    // 7. Output taps.
    double lp = 0.0;
    double bp = 0.0;
    double hp = 0.0;

    if (use18dB)
    {
        lp = y3;
        bp = 3.0 * (y1 - 2.0 * y2 + y3);
        hp = u - 3.0 * y1 + 3.0 * y2 - y3;
    }
    else
    {
        lp = y4;
        bp = 4.0 * (y2 - 2.0 * y3 + y4);
        hp = u - 4.0 * y1 + 6.0 * y2 - 4.0 * y3 + y4;
    }

    // 8. Continuous filter morph: LP -> BP -> HP.
    double m = std::clamp(morph, 0.0, 2.0);

    if (m <= 1.0)
    {
        return (1.0 - m) * lp + m * bp;
    }

    double t = m - 1.0;
    return (1.0 - t) * bp + t * hp;
}