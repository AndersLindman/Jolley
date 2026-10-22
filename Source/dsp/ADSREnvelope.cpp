#include "ADSREnvelope.h"
#include <cmath>
#include <algorithm>

ADSREnvelope::ADSREnvelope(double sr)
    : sampleRate(sr),
      state(State::Idle),
      attackTime(0.001),     // Neutral minimal fallback; overridden immediately by APVTS sync
      decayTime(0.001),      // Neutral minimal fallback
      sustainLevel(0.0),     // Neutral fallback
      releaseTime(0.001),    // Neutral fallback
      attackCurve(0.0),      // Neutral fallback
      decayCurve(0.0),       // Neutral fallback
      releaseCurve(0.0),     // Neutral fallback
      output(0.0),
      sampleCount(0),
      segmentSamples(0),
      startVal(0.0),
      targetVal(0.0),
      coeff(1.0),
      endVal(0.0),
      phase(1.0),
      isLinear(false),
      linearInc(0.0)
{
}

void ADSREnvelope::setSampleRate(double rate)
{
    sampleRate = rate;
}

void ADSREnvelope::noteOn()
{
    startSegment(State::Attack);
}

void ADSREnvelope::noteOff()
{
    if (state != State::Idle)
    {
        startSegment(State::Release);
    }
}

void ADSREnvelope::setParameters(const Parameters& params)
{
    // Remaps UI range [uiMin, 1.0] and applies a smooth power curve (taper)
    auto mapTimeToSeconds = [](double normalized, double uiMin, double minSec, double maxSec, double curveExponent = 2.0) {
        // Shift and scale the UI's restricted range down to a clean 0.0 -> 1.0
        double remapped = (normalized - uiMin) / (1.0 - uiMin);
        remapped = std::clamp(remapped, 0.0, 1.0);

        // Apply a power curve to shape the taper:
        double curved = std::pow(remapped, curveExponent);

        // Linear interpolation across the target time range
        return minSec + curved * (maxSec - minSec);
    };

    if (params.attack >= 0.0)
        attackTime = mapTimeToSeconds(params.attack, 0.02, 0.001, 10.0);

    if (params.decay >= 0.0)         
        decayTime = mapTimeToSeconds(params.decay, 0.02, 0.001, 20.0);

    if (params.sustain >= 0.0)       
        sustainLevel = std::clamp(params.sustain, 0.0, 1.0);

    if (params.release >= 0.0)       
        releaseTime = mapTimeToSeconds(params.release, 0.02, 0.001, 10.0);

    if (params.attackSlope > -900.0)  attackCurve = params.attackSlope;
    if (params.decaySlope > -900.0)   decayCurve = params.decaySlope;
    if (params.releaseSlope > -900.0) releaseCurve = params.releaseSlope;
}

void ADSREnvelope::startSegment(State nextState)
{
    state = nextState;
    sampleCount = 0;
    startVal = output; // Capture current level to prevent clicks

    double time = 0.01;
    double curve = 3.0;

    switch (nextState)
    {
        case State::Attack:
            time = attackTime;
            curve = attackCurve;
            targetVal = 1.0;
            break;

        case State::Decay:
            time = decayTime;
            curve = decayCurve;
            targetVal = sustainLevel;
            break;

        case State::Release:
            time = releaseTime;
            curve = releaseCurve;
            targetVal = 0.0;
            break;

        default:
            break;
    }

    segmentSamples = std::max(1, static_cast<int>(std::round(time * sampleRate)));

    if (std::abs(curve) < 1e-4)
    {
        // Linear Mode
        isLinear = true;
        linearInc = 1.0 / static_cast<double>(segmentSamples);
    }
    else
    {
        // Exponential Mode
        isLinear = false;
        coeff = std::exp(-curve / static_cast<double>(segmentSamples));
        endVal = std::exp(-curve);
        phase = 1.0; // Phase decays from 1.0 down to endVal
    }
}

double ADSREnvelope::processSample()
{
    if (state == State::Idle)
    {
        return 0.0;
    }

    if (state == State::Sustain)
    {
        output = sustainLevel;
        return output;
    }

    // Calculate normalized curve position (1.0 -> 0.0)
    double raw = 0.0;
    if (isLinear)
    {
        raw = 1.0 - (static_cast<double>(sampleCount) * linearInc);
    }
    else
    {
        raw = (phase - endVal) / (1.0 - endVal);
        phase *= coeff;
    }

    // Clamp raw to [0, 1] range against rounding drift
    raw = std::clamp(raw, 0.0, 1.0);

    // Unified interpolation: startVal -> targetVal
    output = targetVal + (startVal - targetVal) * raw;

    sampleCount++;

    // State Machine Transitions
    if (sampleCount >= segmentSamples)
    {
        if (state == State::Attack)
        {
            startSegment(State::Decay);
        }
        else if (state == State::Decay)
        {
            state = State::Sustain;
        }
        else if (state == State::Release)
        {
            state = State::Idle;
            output = 0.0;
        }
    }

    return output;
}