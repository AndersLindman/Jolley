#include "FMProcessor.h"
#include "../../ParamIds.h"
#include <algorithm>
#include <cmath>

// Define static array storage matching the header declaration
constexpr double FMProcessor::fmRatios[numRatios];

FMProcessor::FMProcessor()
    : active(false),
      osc3Ratio(1.0),
      depth3to2(0.0),
      osc3Feedback(0.0),
      osc2Ratio(1.0),
      depth2to1(0.0),
      masterAmount(0.0),
      lastOsc3Sample(0.0)
{
}

void FMProcessor::setParam(const juce::String& id, double normalized)
{
    if (id == ParamIDs::FM_ACTIVE.data())
    {
        active = (normalized > 0.5f);
    }
    else if (id == ParamIDs::FM_DEPTH_3TO2.data())
    {
        depth3to2 = normalized * 30.0;
    }
    else if (id == ParamIDs::FM_OSC3_FEEDBACK.data())
    {
        osc3Feedback = normalized * 0.95;
    }
    else if (id == ParamIDs::FM_DEPTH_2TO1.data())
    {
        depth2to1 = normalized * 30.0;
    }
    else if (id == ParamIDs::FM_MASTER_AMOUNT.data())
    {
        masterAmount = normalized;
    }
    else if (id == ParamIDs::FM_OSC2_RATIO.data())
    {
        int index = std::clamp(
            static_cast<int>(std::round(normalized * (numRatios - 1))),
            0,
            numRatios - 1
        );
        osc2Ratio = fmRatios[index];
    }
    else if (id == ParamIDs::FM_OSC3_RATIO.data())
    {
        int index = std::clamp(
            static_cast<int>(std::round(normalized * static_cast<double>(numRatios - 1))),
            0,
            numRatios - 1
        );
        osc3Ratio = fmRatios[index];
    }
}

void FMProcessor::reset()
{
    lastOsc3Sample = 0.0;
}