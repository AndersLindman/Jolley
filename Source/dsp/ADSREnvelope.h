#pragma once
#include <JuceHeader.h>

class ADSREnvelope
{
public:
    enum State
    {
        Idle = 0,
        Attack = 1,
        Decay = 2,
        Sustain = 3,
        Release = 4
    };

    struct Parameters
    {
        double attack = -1.0;
        double decay = -1.0;
        double sustain = -1.0;
        double release = -1.0;
        double attackSlope = -999.0;
        double decaySlope = -999.0;
        double releaseSlope = -999.0;
    };

    ADSREnvelope(double sampleRate = 44100.0);
    ~ADSREnvelope() = default;

    void setSampleRate(double rate);
    void noteOn();
    void noteOff();
    void setParameters(const Parameters& params);
    double processSample();

    State getState() const { return state; }
    double getCurrentOutput() const { return output; }

private:
    void startSegment(State nextState);

    double sampleRate;
    State state;

    // Parameters
    double attackTime;
    double decayTime;
    double sustainLevel;
    double releaseTime;

    double attackCurve;
    double decayCurve;
    double releaseCurve;

    // Runtime state
    double output;
    int sampleCount;
    int segmentSamples;

    // Segment interpolation state
    double startVal;
    double targetVal;

    // Exponential coefficients
    double coeff;
    double endVal;
    double phase;
    bool isLinear;
    double linearInc;
};