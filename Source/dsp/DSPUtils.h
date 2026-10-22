#pragma once

#include <algorithm>

namespace JolleyDSP
{
    /**
     * Standard Soft Clipper
     */
    constexpr inline double softClip(double x)
    {
        double z = std::clamp(x, -1.875, 1.875) / 1.875;
        double z2 = z * z;
        return z * (1.875 - z2 * (1.25 - 0.375 * z2));
    }
}