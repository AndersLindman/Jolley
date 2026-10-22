#pragma once

#include <JuceHeader.h>

class FMProcessor
{
public:
    FMProcessor();

    void setParam(const juce::String& id, double normalized);
    void reset();

    // Inline Getters for SynthVoice access
    bool isActive() const { return active; }
    double getOsc2Ratio() const { return osc2Ratio; }
    double getOsc3Ratio() const { return osc3Ratio; }
    double getMasterAmount() const { return masterAmount; }
    double getOsc3Feedback() const { return osc3Feedback; }
    double getDepth3to2() const { return depth3to2; }
    double getDepth2to1() const { return depth2to1; }

private:
    bool active;
    double osc3Ratio;
    double depth3to2;
    double osc3Feedback;
    double osc2Ratio;
    double depth2to1;
    double masterAmount;
    double lastOsc3Sample;

    static constexpr int numRatios = 13;
    static constexpr double fmRatios[numRatios] = { 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0 };
};