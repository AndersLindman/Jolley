#pragma once
#include <JuceHeader.h>
#include "ZDFfilter.h"
#include "LadderFilter.h"
#include "DiodeFilter.h"

class MultimodeFilter
{
public:
    MultimodeFilter(double sampleRate = 44100.0);
    ~MultimodeFilter() = default;

    void setSampleRate(double rate);
    void reset();
    double process(double inSample, int type, double cutoffHz, double resonance, double drive, double feedback, double morph);

private:
    double tanhApprox(double x);

    double sampleRate;
    ZDFfilter zdf;
    LadderFilter ladder;
    DiodeFilter diode;

    double zdfLastOutput;
    double ladderLastOutput;
    double diodeLastOutput;
};