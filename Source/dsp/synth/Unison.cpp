#include "Unison.h"
#include <cmath>
#include <algorithm>

Unison::Unison(int maxVoicesCount)
    : maxVoices(maxVoicesCount)
{
    setupVoices();
}

void Unison::setupVoices()
{
    voices.clear();
    voices.resize(maxVoices);
}

int Unison::updateConfig(int voiceCount, double detuneCents, double spreadWidth)
{
    const int count = std::max(1, std::min(maxVoices, voiceCount));
    const double TWO_PI = juce::MathConstants<double>::twoPi;

    constexpr double WARMTH_POWER = 10.0;

    // Use JUCE's thread-safe random generator helper
    juce::Random& rng = juce::Random::getSystemRandom();

    for (int i = 0; i < count; ++i)
    {
        // 1. Base Linear Spread (-1 to 1)
        double norm = (count == 1) ? 0.0 : (2.0 * static_cast<double>(i) / static_cast<double>(count - 1)) - 1.0;

        // 2. Apply Power Spacing using clean sign copysign math
        double curvedNorm = std::copysign(std::pow(std::abs(norm), WARMTH_POWER), norm);

        // Calculate final offsets
        voices[i].detuneOffset = curvedNorm * detuneCents;
        voices[i].panOffset = curvedNorm * spreadWidth;

        // 3. Equidistant Phase Spread
        voices[i].phaseOffset = (static_cast<double>(i) / static_cast<double>(count)) * TWO_PI;

        // 4. Initialize Smooth Drift safely
        double randomVal = rng.nextDouble(); // Returns 0.0 to 1.0 thread-safely
        voices[i].driftTarget = (randomVal - 0.5) * (detuneCents * 0.4); // Max 40% extra drift
        voices[i].currentDrift = 0.0;
    }

    // Clear out inactive voices beyond the current active count
    for (int i = count; i < maxVoices; ++i)
    {
        voices[i].detuneOffset = 0.0;
        voices[i].panOffset = 0.0;
        voices[i].phaseOffset = 0.0;
        voices[i].driftTarget = 0.0;
        voices[i].currentDrift = 0.0;
    }

    return count;
}

const UnisonVoiceConfig* Unison::getVoice(int index) const
{
    if (index >= 0 && index < maxVoices)
    {
        return &voices[index];
    }
    return nullptr;
}