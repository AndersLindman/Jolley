#pragma once
#include <JuceHeader.h>
#include <vector>
#include <unordered_map>
#include "../../ParamIds.h"
#include "SynthVoice.h"
#include "FMProcessor.h"

class SynthEngine
{
public:
    SynthEngine(int numVoices = 8);
    ~SynthEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void noteOn(int note, double velocity);
    void noteOff(int note);
    void allNotesOff(bool immediate);
    void setPitchBend(double normalizedValue);
    void setModWheel(double normalizedValue);
    void setParam(const juce::String& id, double normalized); // Changed from float
    void processSubBlock(double* outputLeft, double* outputRight, int numSamples);

private:
    double sampleRate;
    int maxVoices;
    std::vector<SynthVoice> voices;
    FMProcessor fmProcessor;

    // --- Unison & Voice Configuration State (Neutral Placeholders) ---
    int unisonVoices = 1;          // Minimum structural floor
    double unisonDetune = 0.0;     // Layout default is 15.0
    double unisonSpread = 0.0;     // Layout default is 0.8
    double masterVolume = 0.0;     // Layout default is 0.7

    // Maps MIDI note -> array of voice indices currently playing it
    std::unordered_map<int, std::vector<int>> activeNoteGroups;

    // Pre-allocated scratch buffers for mixing to prevent real-time memory allocations
    std::vector<double> mixBlockLeft;
    std::vector<double> mixBlockRight;

    double globalLastNote = -1.0; // Timeline anchor for portamento/glide
    int voiceMode = 0; // 0: Poly, 1: Mono, 2: Legato
    std::vector<int> heldNoteStack; // Tracks currently held MIDI notes for mono/legato priority
    int samplesUntilGlobalReset = -1; // -1 means inactive/expired
};