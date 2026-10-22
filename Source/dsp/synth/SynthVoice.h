#pragma once
#include <JuceHeader.h>
#include "../../ParamIds.h"
#include "../ADSREnvelope.h"
#include "../MultimodeFilter.h"
#include "../DSFOscillator.h"
#include "FMProcessor.h"
#include "Unison.h"
#include "../LFO.h"
#include "../TubeDrive.h"

struct UnisonLayer {
    double cents = 0.0;
    double panOffset = 0.0;
    double lastOsc3Sample = 0.0;
    std::array<DSFOscillator, 3> oscillators;
    FMProcessor fmProcessor;
};

struct BaseOscParams {
    double baseShape = 0.0;
    double rolloff = 0.0;
    double level = 0.0;
    int octave = 0;
    int semitone = 0;
    double fine = 0.0;
    double pan = 0.0;
    double shape = 0.0;
};

struct FilterParams {
    int type = 0;
    double cutoff = 0.0;
    double resonance = 0.0;
    double drive = 0.0;
    double feedback = 0.0;
    double morph = 0.0;
    double envAmount = 0.0;
};

class SynthVoice
{
public:
    SynthVoice();
    ~SynthVoice() = default;

    void prepare(double newSampleRate);
    void noteOn(int midiNote, double rawVelocity, bool shouldRetiggerEnvelopes = true);
    void noteOff();
    void setParam(const juce::String& id, double normalized);
    void setPortamentoSource(double lastNote);
    void processBlock(double* outputBufferLeft, double* outputBufferRight, int numSamples);
    bool isActive() const { return active; }

    void setPitchBend(double normalizedValue)
    {
        pitchBendVal = normalizedValue;
    }

    void setModWheel(double normalizedValue)
    {
        modWheelVal = std::clamp(normalizedValue, 0.0, 1.0);
    }

    void stopImmediately()
    {
        active = false;
    }

private:
    void initUnisonLayers(double newSampleRate);
    void updateUnisonOffsets();
    void updateFrequencies();
    void updateOscillatorParams(int oscIndex);

    double sampleRate = 0.0;
    std::vector<BaseOscParams> baseOscParams;
    int unisonVoices = 0;
    double unisonDetune = 0.0;
    double unisonSpread = 0.0;
    int maxUnisonVoices = 0;
    Unison unisonManager;
    std::vector<UnisonLayer> unisonLayers;

    ADSREnvelope ampEnv;
    ADSREnvelope filterEnv;
    ADSREnvelope pitchEnv;
    ADSREnvelope modEnv;

    MultimodeFilter filterLeft;
    MultimodeFilter filterRight;
    FMProcessor fmProcessor;

    FilterParams filterParams;

    double lastOsc3Sample = 0.0;
    int note = -1;
    double velocity = 0.0;
    double voiceVelocityGain = 0.0;
    bool active = false;
    bool syncToOsc2 = false;
    bool syncToOsc3 = false;

    std::array<double, 3> filterEnvToOsc;
    std::array<double, 3> pitchEnvToOsc;
    double pitchEnvAmount = 0.0;
    std::array<double, 3> modEnvToOsc;
    double modEnvToFm = 0.0;
    double modEnvToShape1 = 0.0;
    double modEnvToShape2 = 0.0;
    double modEnvToShape3 = 0.0;

    double currentNote = -1.0;
    double targetNote = -1.0;
    double portamentoSource = -1.0;
    double glideTimeMs = 0.0;

    double pitchBendRange = 0.0;
    double pitchBendVal = 0.0;

    LFO lfo1;
    LFO lfo2;

    double lfo1Rate = 0.0;
    double lfo1Shape = 0.0;
    double lfo1Depth = 0.0;
    bool lfo1DestOsc1Pitch = false;
    bool lfo1DestOsc1Shape = false;
    bool lfo1DestCutoff = false;

    double lfo2Rate = 0.0;
    double lfo2Shape = 0.0;
    double lfo2Depth = 0.0;
    bool lfo2DestOsc2Pitch = false;
    bool lfo2DestOsc2Shape = false;
    bool lfo2DestFMAmount = false;

    double velocityDepth = 0.0;
    double voiceDrift = 0.0;
    double voiceDriftCents = 0.0;
    double modWheelCutoffDepth = 0.0;
    double modWheelVal = 0.0;

    TubeDrive tubeDriveLeft;
    TubeDrive tubeDriveRight;
};