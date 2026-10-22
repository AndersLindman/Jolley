#pragma once
#include <cmath>
#include <algorithm>

class PitchEnvelope {
public:
    void setSampleRate(double sr) {
        sampleRate = sr;
    }

    void setParameters(float attack, float attackSlope,
                       float decay, float decaySlope,
                       float sustain,
                       float release, float releaseSlope) {
        attackTime = std::max(0.0001f, attack);
        aSlope = attackSlope;
        
        decayTime = std::max(0.0001f, decay);
        dSlope = decaySlope;
        
        sustainLevel = std::clamp(sustain, 0.0f, 1.0f);
        
        releaseTime = std::max(0.0001f, release);
        rSlope = releaseSlope;
    }

    void noteOn() {
        currentState = State::Attack;
        startTime = currentValue;
        targetTime = 1.0f;
        stageProgress = 0.0;
        stageDuration = static_cast<double>(attackTime * sampleRate);
    }

    void noteOff() {
        currentState = State::Release;
        startTime = currentValue;
        targetTime = 0.0f;
        stageProgress = 0.0;
        stageDuration = static_cast<double>(releaseTime * sampleRate);
    }

    float getNextSample() {
        if (currentState == State::Idle) return 0.0f;

        // Advance progress
        stageProgress += 1.0;
        double t = std::clamp(stageProgress / std::max(1.0, stageDuration), 0.0, 1.0);

        // Apply curve shape based on slope parameter
        float shapedT = applySlope(t, currentSlope());

        // Interpolate current value for the stage
        currentValue = startTime + (targetTime - startTime) * shapedT;

        // Check for stage completion and transition
        if (t >= 1.0) {
            if (currentState == State::Attack) {
                currentState = State::Decay;
                startTime = 1.0f;
                targetTime = sustainLevel;
                stageProgress = 0.0;
                stageDuration = static_cast<double>(decayTime * sampleRate);
            }
            else if (currentState == State::Decay) {
                currentState = State::Sustain;
                currentValue = sustainLevel;
            }
            else if (currentState == State::Release) {
                currentState = State::Idle;
                currentValue = 0.0f;
            }
        }

        return currentValue;
    }

    bool isActive() const {
        return currentState != State::Idle;
    }

private:
    enum class State { Idle, Attack, Decay, Sustain, Release };
    State currentState = State::Idle;

    double sampleRate = 44100.0;
    float currentValue = 0.0f;

    // Raw parameter values
    float attackTime = 0.01f;
    float aSlope = 3.0f;
    float decayTime = 0.3f;
    float dSlope = 3.0f;
    float sustainLevel = 0.5f;
    float releaseTime = 0.4f;
    float rSlope = 3.0f;

    // Stage tracking state
    double stageProgress = 0.0;
    double stageDuration = 1.0;
    float startTime = 0.0f;
    float targetTime = 0.0f;

    float currentSlope() const {
        switch (currentState) {
            case State::Attack:  return aSlope;
            case State::Decay:   return dSlope;
            case State::Release: return rSlope;
            default:             return 0.0f;
        }
    }

    // Maps linear progress (0 to 1) into a curved progression using the slope parameter
    float applySlope(double t, float slope) {
        if (std::abs(slope) < 0.01f) return static_cast<float>(t); // Linear
        
        // Exponential curve mapping based on slope (-20 to 20)
        double curve = std::exp(slope * 0.1);
        return static_cast<float>((std::pow(curve, t) - 1.0) / (curve - 1.0));
    }
};