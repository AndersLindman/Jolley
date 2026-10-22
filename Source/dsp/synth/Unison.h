#pragma once
#include <JuceHeader.h>
#include <vector>

struct UnisonVoiceConfig {
    double detuneOffset = 0.0;    // Cents offset
    double panOffset = 0.0;       // -1 (Left) to +1 (Right)
    double phaseOffset = 0.0;     // Starting phase for fatness
    double driftTarget = 0.0;     // Random drift target
    double currentDrift = 0.0;    // Current smoothed drift
};

class Unison
{
public:
    Unison(int maxVoices = 8);
    ~Unison() = default;

    int updateConfig(int voiceCount, double detuneCents, double spreadWidth);
    const UnisonVoiceConfig* getVoice(int index) const;

private:
    void setupVoices();

    int maxVoices;
    std::vector<UnisonVoiceConfig> voices;
};