#include "SynthEngine.h"
#include <algorithm>

SynthEngine::SynthEngine(int numVoices)
    : maxVoices(numVoices), 
      voices(numVoices), 
      sampleRate(44100.0),
      unisonVoices(1), 
      unisonDetune(15.0), 
      unisonSpread(0.8), 
      masterVolume(1.0),
      voiceMode(0), 
      globalLastNote(-1.0),
      samplesUntilGlobalReset(-1)
{
    heldNoteStack.reserve(maxVoices);
}

void SynthEngine::prepare(double newSampleRate, int samplesPerBlock, int numChannels)
{
    juce::ignoreUnused(numChannels);
    sampleRate = newSampleRate;
    samplesUntilGlobalReset = -1;

    for (auto& voice : voices)
    {
        voice.prepare(sampleRate);
    }

    mixBlockLeft.resize(samplesPerBlock, 0.0);
    mixBlockRight.resize(samplesPerBlock, 0.0);
}

void SynthEngine::noteOn(int note, double velocity)
{
    if (globalLastNote != -1.0 && activeNoteGroups.empty() && heldNoteStack.empty())
    {
        if (samplesUntilGlobalReset <= 0)
        {
            globalLastNote = -1.0;
            samplesUntilGlobalReset = -1;
        }
    }
    samplesUntilGlobalReset = -1;

    heldNoteStack.erase(std::remove(heldNoteStack.begin(), heldNoteStack.end(), note), heldNoteStack.end());
    heldNoteStack.push_back(note);

    SynthVoice* targetVoice = nullptr;
    int voiceIndex = 0;
    bool shouldRetigger = true;

    if (voiceMode == 0) // --- POLY MODE ---
    {
        if (activeNoteGroups.find(note) != activeNoteGroups.end())
        {
            noteOff(note); // Stop existing note before retriggering
        }

        // Find an inactive voice
        auto it = std::find_if(voices.begin(), voices.end(), [](const SynthVoice& v) {
            return !v.isActive();
        });

        if (it != voices.end())
        {
            voiceIndex = static_cast<int>(std::distance(voices.begin(), it));
            targetVoice = &(*it);
        }
        else
        {
            // Voice stealing fallback: take the first voice (you could improve this to steal the oldest released voice)
            voiceIndex = 0;
            targetVoice = &voices[0];

            for (auto& pair : activeNoteGroups)
            {
                auto& indices = pair.second;
                indices.erase(std::remove(indices.begin(), indices.end(), voiceIndex), indices.end());
            }
        }

        activeNoteGroups[note] = { voiceIndex };
    }
    else // --- MONO / LEGATO MODE ---
    {
        voiceIndex = 0;
        targetVoice = &voices[0];

        // Legato mode: do not retrigger envelopes if other notes are still held
        if (voiceMode == 2 && heldNoteStack.size() > 1)
        {
            shouldRetigger = false;
        }
    }

    targetVoice->setPortamentoSource(globalLastNote);
    targetVoice->noteOn(note, velocity, shouldRetigger);

    globalLastNote = static_cast<double>(note);
}

void SynthEngine::noteOff(int note)
{
    heldNoteStack.erase(std::remove(heldNoteStack.begin(), heldNoteStack.end(), note), heldNoteStack.end());

    if (voiceMode == 0) // --- POLY MODE ---
    {
        auto it = activeNoteGroups.find(note);
        if (it != activeNoteGroups.end())
        {
            for (int idx : it->second)
            {
                if (idx >= 0 && idx < static_cast<int>(voices.size()))
                {
                    voices[idx].noteOff(); // Triggers release phase
                }
            }
            activeNoteGroups.erase(it); // Note is no longer "held", even if sound is releasing
        }

        if (activeNoteGroups.empty())
        {
            // 500 ms expressed in samples (gives portamento time to reset)
            samplesUntilGlobalReset = static_cast<int>(sampleRate * 0.5);
        }
    }
    else // --- MONO / LEGATO MODE ---
    {
        SynthVoice* monoVoice = &voices[0];

        if (!heldNoteStack.empty())
        {
            int previousNote = heldNoteStack.back();
            monoVoice->setPortamentoSource(static_cast<double>(note));
            monoVoice->noteOn(previousNote, 1.0f, false); // false = legato, no retrigger
            globalLastNote = static_cast<double>(previousNote);
            samplesUntilGlobalReset = -1;
        }
        else
        {
            monoVoice->noteOff();
            samplesUntilGlobalReset = static_cast<int>(sampleRate * 0.5);
        }
    }
}

void SynthEngine::setPitchBend(double normalizedValue)
{
    for (auto& voice : voices)
    {
        voice.setPitchBend(normalizedValue);
    }
}

void SynthEngine::setModWheel(double normalizedValue)
{
    for (auto& voice : voices)
    {
        voice.setModWheel(normalizedValue);
    }
}

void SynthEngine::allNotesOff(bool immediate)
{
    for (auto& voice : voices)
    {
        if (immediate)
        {
            voice.stopImmediately(); // Hard reset for CC 124 / Reaper F3
        }
        else
        {
            voice.noteOff(); // Normal release for CC 123
        }
    }
    
    activeNoteGroups.clear();
    heldNoteStack.clear();
    globalLastNote = -1.0;
    samplesUntilGlobalReset = -1;
}

void SynthEngine::setParam(const juce::String& id, double normalized)
{
    auto matchesId = [&id](std::string_view sv) {
        return id == juce::String(sv.data(), sv.size());
    };

    if (matchesId(ParamIDs::UNISON_VOICES))
    {
        const int min = 1;
        const int max = 8;
        unisonVoices = std::clamp(static_cast<int>(std::round(min + normalized * (max - min))), min, max);
    }
    else if (matchesId(ParamIDs::UNISON_DETUNE))
    {
        unisonDetune = static_cast<double>(normalized) * 100.0;
    }
    else if (matchesId(ParamIDs::UNISON_SPREAD))
    {
        unisonSpread = static_cast<double>(normalized);
    }
    else if (matchesId(ParamIDs::MASTER_VOLUME))
    {
        masterVolume = static_cast<double>(normalized);
    }
    else if (matchesId(ParamIDs::VOICE_MODE))
    {
        voiceMode = static_cast<int>(std::round(normalized * 2.0)); // 0 = Poly, 1 = Mono, 2 = Legato
    }

    // Forward the normalized [0, 1] value to all voices
    for (auto& voice : voices)
    {
        voice.setParam(id, normalized);
    }
}

void SynthEngine::processSubBlock(double* outputLeft, double* outputRight, int numSamples)
{
    if (numSamples <= 0)
        return;

    // Decrement the portamento reset countdown safely on the audio thread
    if (samplesUntilGlobalReset > 0)
    {
        samplesUntilGlobalReset -= numSamples;
        if (samplesUntilGlobalReset <= 0)
        {
            samplesUntilGlobalReset = 0;
            globalLastNote = -1.0;
        }
    }

    // Ensure scratch buffers are large enough
    if (numSamples > static_cast<int>(mixBlockLeft.size()) ||
        numSamples > static_cast<int>(mixBlockRight.size()))
    {
        mixBlockLeft.resize(numSamples, 0.0);
        mixBlockRight.resize(numSamples, 0.0);
    }

    std::fill(mixBlockLeft.begin(), mixBlockLeft.begin() + numSamples, 0.0);
    std::fill(mixBlockRight.begin(), mixBlockRight.begin() + numSamples, 0.0);

    // Render active voices into temporary mix buffers
    for (int v = 0; v < maxVoices; v++)
    {
        if (voices[v].isActive())
        {
            voices[v].processBlock(mixBlockLeft.data(), mixBlockRight.data(), numSamples);
        }
    }

    const double masterVol = masterVolume;

    // Sum into output channels
    for (int i = 0; i < numSamples; ++i)
    {
        outputLeft[i] += mixBlockLeft[i] * 0.25 * masterVol;
        if (outputRight != nullptr)
        {
            outputRight[i] += mixBlockRight[i] * 0.25 * masterVol;
        }
    }
}