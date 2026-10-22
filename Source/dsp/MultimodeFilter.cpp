#include "MultimodeFilter.h"
#include <cmath>
#include <algorithm>

MultimodeFilter::MultimodeFilter(double sr)
    : sampleRate(sr),
      zdfLastOutput(0.0),
      ladderLastOutput(0.0),
      diodeLastOutput(0.0)
{
}

void MultimodeFilter::setSampleRate(double rate)
{
    sampleRate = rate;
}

void MultimodeFilter::reset()
{
    zdf.reset();
    ladder.reset();
    diode.reset();
    zdfLastOutput = 0.0;
    ladderLastOutput = 0.0;
    diodeLastOutput = 0.0;
}

double MultimodeFilter::tanhApprox(double x)
{
//    double clamped = std::clamp(x, -3.0, 3.0);
//    return clamped * (27.0 + clamped * clamped) / (27.0 + 9.0 * clamped * clamped);
//return std::tanh(x);
    double z = x / 1.875;
    if (z <= -1.0) return -1.0;
    if (z >= 1.0) return 1.0;
    double z2 = z * z;
    return z * (1.875 - z2 * (1.25 - 0.375 * z2));
}


double MultimodeFilter::process(double inSample, int type, double cutoffHz, double resonance, double drive, double feedback, double morph)
{
    // --- 1. Compute Drive Scale (Proportional Attenuation) ---
    // Inverse scaling based on the input boost factor (1.0 + drive * 4.0).
    // Square root prevents over-attenuating, balancing perceived loudness.
    //double driveScale = 1.0 / std::sqrt(1.0 + drive * 3.0);
    double driveScale = 1.0 / (1.0 + drive * 3.0);

    // 2. Apply Input Drive
    double drivenInput = inSample * (1.0 + drive * 4.0);
    if (drive > 0.0)
    {
        // drivenInput = tanhApprox(drivenInput);
    }

    double output = 0.0;

    switch (type)
    {
        case 0: // 18dB ZDF Filter
        {
            double resGain = resonance * 8.0;
     //       double feedbackSignal = tanhApprox(zdfLastOutput) * feedback * 2.0;
            double feedbackSignal = tanhApprox(zdfLastOutput) * feedback;
            output = zdf.process(drivenInput + feedbackSignal, cutoffHz, resGain, sampleRate, morph * 2.0);
            zdfLastOutput = output;
            break;
        }

        case 1: // 24dB Moog Ladder Filter
        {
            double resGain = resonance * 3.5;
            double feedbackSignal = tanhApprox(ladderLastOutput) * feedback;
            output = ladder.process(drivenInput + feedbackSignal, cutoffHz, resGain, sampleRate, morph * 2.0, true);
            ladderLastOutput = output;
            break;
        }

        case 2: // Clean Diode Filter (TB-303 18dB style)
        {
            double resGain = resonance * 3.0;
            double feedbackSignal = tanhApprox(diodeLastOutput) * feedback;
            double diodeDrive = std::max(1.5, 1.0 + drive * 3.0);

            output = diode.process(
                drivenInput + feedbackSignal,
                cutoffHz,
                resGain,
                sampleRate,
                diodeDrive,
                morph * 2.0,
                true // 18dB authentic tap
            );
            diodeLastOutput = output;
            break;
        }

        default:
            output = drivenInput;
            break;
    }

    return output * driveScale;
}