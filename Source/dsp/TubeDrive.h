#pragma once

#include <algorithm>
#include <cmath>
#include "DSPUtils.h"

class TubeDrive
{
public:
    TubeDrive() = default;

    void setDrive(double normalizedDrive)
    {
        DBG("targetDrive" << normalizedDrive);
        targetDrive = std::clamp(normalizedDrive, 0.0, 1.0);
    }

    void reset()
    {
        previousInput = 0.0;
        hasPreviousInput = false;
    }

    inline double processSample(double input)
    {
        if (targetDrive <= 0.0)
        {
            // Avoid using stale ADAA state when drive is re-enabled.
            hasPreviousInput = false;
            return input;
        }

        const double drive = (1.0 + targetDrive) * 2.0;
        const double bias = 0.07 * drive * targetDrive;
        const double biasedInput = input * drive + bias;

        // First-order ADAA waveshaping.
        double shaped;

        if (!hasPreviousInput)
        {
            // Initialize without introducing a startup transient.
            shaped = JolleyDSP::softClip(biasedInput);
            hasPreviousInput = true;
        }
        else
        {
            const double delta = biasedInput - previousInput;

            // Avoid numerical cancellation when samples are very close.
            const double tolerance =
                1.0e-8 * std::max(
                    1.0,
                    std::max(std::abs(biasedInput),
                             std::abs(previousInput)));

            if (std::abs(delta) < tolerance)
            {
                const double midpoint =
                    0.5 * (biasedInput + previousInput);

                shaped = JolleyDSP::softClip(midpoint);
            }
            else
            {
                shaped =
                    (antiderivative(biasedInput)
                     - antiderivative(previousInput)) / delta;
            }
        }

        previousInput = biasedInput;

        // Remove the static offset introduced by the bias.
        const double biasReference = JolleyDSP::softClip(bias);
        const double output = shaped - biasReference;

        // Preserve the existing gain reduction.
        const double gainReduction = 4.2 / (6.0 + targetDrive);

        return output * gainReduction;
    }

private:
    double targetDrive = 0.0;

    double previousInput = 0.0;
    bool hasPreviousInput = false;

    // Analytic antiderivative of clip().
    // Inside |x| < 1.875, integrate the polynomial.
    // Outside that region, clip() is constant, so the
    // antiderivative continues linearly.
    static inline double antiderivative(double x)
    {
        constexpr double limit = 1.875;
        constexpr double boundaryValue = 1.2890625;

        const double absX = std::abs(x);

        if (absX >= limit)
            return boundaryValue + absX - limit;

        const double z = x / limit;
        const double z2 = z * z;
        const double z4 = z2 * z2;
        const double z6 = z4 * z2;

        return limit *
            (0.9375 * z2
             - 0.3125 * z4
             + 0.0625 * z6);
    }
};
