#include "SynthVoice.h"
#include <cmath>
#include <algorithm>

SynthVoice::SynthVoice()
    : sampleRate(44100.0),
      baseOscParams(3),
      unisonVoices(1),
      unisonDetune(0.0),
      unisonSpread(0.0),
      maxUnisonVoices(8),
      unisonManager(maxUnisonVoices),
      lastOsc3Sample(0.0),
      note(-1),
      velocity(0.0f),
      voiceVelocityGain(1.0),
      active(false),
      syncToOsc2(false),
      syncToOsc3(false),
      filterEnvToOsc({0.0, 0.0, 0.0}),
      pitchEnvToOsc({0.0, 0.0, 0.0}),
      pitchEnvAmount(0.5),
      modEnvToOsc({0.0, 0.0, 0.0}),
      modEnvToFm(0.0),
      modEnvToShape1(0.0),
      modEnvToShape2(0.0),
      modEnvToShape3(0.0),
      currentNote(-1.0),
      targetNote(-1.0),
      portamentoSource(-1.0),
      glideTimeMs(0.0),
      pitchBendRange(2.0),
      pitchBendVal(0.0),
      lfo1Rate(0.0),
      lfo1Shape(0.0),
      lfo1Depth(0.0),
      lfo1DestOsc1Pitch(false),
      lfo1DestOsc1Shape(false),
      lfo1DestCutoff(false),
      lfo2Rate(0.0),
      lfo2Shape(0.0),
      lfo2Depth(0.0),
      lfo2DestOsc2Pitch(false),
      lfo2DestOsc2Shape(false),
      lfo2DestFMAmount(false),
      velocityDepth(0.0),
      voiceDrift(0.0),
      voiceDriftCents(0.0),
      modWheelCutoffDepth(0.0),
      modWheelVal(0.0)
{
    baseOscParams[0] = { 0.0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0 };
    baseOscParams[1] = { 0.0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0 };
    baseOscParams[2] = { 0.0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0 };

    initUnisonLayers(sampleRate);
}

void SynthVoice::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    ampEnv.setSampleRate(sampleRate);
    filterEnv.setSampleRate(sampleRate);
    pitchEnv.setSampleRate(sampleRate);
    modEnv.setSampleRate(sampleRate);

    lfo1.setSampleRate(sampleRate);
    lfo2.setSampleRate(sampleRate);

    filterLeft.setSampleRate(sampleRate);
    filterLeft.reset();
    filterRight.setSampleRate(sampleRate);
    filterRight.reset();

    initUnisonLayers(sampleRate);
    updateFrequencies();
}

void SynthVoice::noteOn(int midiNote, double rawVelocity, bool shouldRetiggerEnvelopes)
{
    note = midiNote;
    targetNote = static_cast<double>(midiNote);

    if (portamentoSource >= 0.0 && glideTimeMs > 0.0)
    {
        currentNote = portamentoSource;
    }
    else
    {
        currentNote = targetNote;
    }

    // Now operating entirely in double precision
    velocity = std::clamp(rawVelocity, 0.0, 1.0);
    voiceVelocityGain = 1.0 - velocityDepth + (velocity * velocityDepth);

    if (voiceDrift > 0.001)
    {
        juce::Random rng;
        voiceDriftCents = (rng.nextDouble() * 2.0 - 1.0) * 6.0 * voiceDrift;
    }
    else
    {
        voiceDriftCents = 0.0;
    }

    active = true;

    if (shouldRetiggerEnvelopes)
    {
        ampEnv.noteOn();
        filterEnv.noteOn();
        pitchEnv.noteOn();
        modEnv.noteOn();

        filterLeft.reset();
        filterRight.reset();
        fmProcessor.reset();

        int activeCount = std::clamp(unisonVoices, 1, maxUnisonVoices);
        for (int u = 0; u < activeCount; ++u)
        {
            unisonLayers[u].lastOsc3Sample = 0.0;
            for (int i = 0; i < 3; ++i)
            {
                unisonLayers[u].oscillators[i].resetPhase();
            }
        }
    }

    updateFrequencies();
}

void SynthVoice::noteOff()
{
    // Triggers the release phase of all envelopes
    ampEnv.noteOff();
    filterEnv.noteOff();
    pitchEnv.noteOff();
    modEnv.noteOff();
}

void SynthVoice::setPortamentoSource(double lastNote)
{
    portamentoSource = lastNote;
}

void SynthVoice::initUnisonLayers(double newSampleRate)
{
    unisonLayers.clear();
    unisonLayers.resize(maxUnisonVoices);

    for (int u = 0; u < maxUnisonVoices; ++u)
    {
        UnisonLayer layer;
        layer.cents = 0.0;
        layer.panOffset = 0.0;
        layer.lastOsc3Sample = 0.0;
        
        for (int i = 0; i < 3; ++i)
        {
            layer.oscillators[i].setSampleRate(newSampleRate);
            layer.oscillators[i].setParams(baseOscParams[i].rolloff, baseOscParams[i].shape);
            layer.oscillators[i].setFrequency(440.0);
        }

        unisonLayers[u] = layer;
    }
    updateUnisonOffsets();
}

void SynthVoice::updateUnisonOffsets()
{
    int activeCount = unisonManager.updateConfig(unisonVoices, unisonDetune, unisonSpread);

    for (int u = 0; u < maxUnisonVoices; ++u)
    {
        auto& layer = unisonLayers[u];
        const auto* voiceConfig = unisonManager.getVoice(u);

        if (u < activeCount && voiceConfig != nullptr)
        {
            layer.cents = voiceConfig->detuneOffset;
            layer.panOffset = voiceConfig->panOffset;
        }
        else
        {
            layer.cents = 0.0;
            layer.panOffset = 0.0;
        }
    }
}

void SynthVoice::updateFrequencies()
{
    if (note < 0) return;

    int activeCount = std::clamp(unisonVoices, 1, maxUnisonVoices);
    
    for (int u = 0; u < activeCount; ++u)
    {
        auto& layer = unisonLayers[u];
        double effectiveNote = currentNote + (layer.cents / 100.0) + (voiceDriftCents / 10.0);
        double baseFreq = 440.0 * std::pow(2.0, (effectiveNote - 69.0) / 12.0);

        for (int i = 0; i < 3; ++i)
        {
            const auto& p = baseOscParams[i];
            double semitoneTotal = static_cast<double>(p.semitone) + (p.fine / 100.0);
            
            double ratio = 1.0;
            if (i == 1) ratio = fmProcessor.getOsc2Ratio();
            else if (i == 2) ratio = fmProcessor.getOsc3Ratio();

            double freq = baseFreq * ratio * std::pow(2.0, static_cast<double>(p.octave) + (semitoneTotal / 12.0));
            layer.oscillators[i].setFrequency(freq);
        }
    }
}

void SynthVoice::updateOscillatorParams(int oscIndex)
{
    const auto& p = baseOscParams[oscIndex];
    for (auto& layer : unisonLayers)
    {
        layer.oscillators[oscIndex].setParams(p.rolloff, p.shape);
    }
}

void SynthVoice::setParam(const juce::String& id, double normalized)
{

    // 1. Unison Parameters
    if (id == ParamIDs::UNISON_VOICES.data())
    {
        int min = 1;
        int max = 8;
        int newCount = std::clamp(static_cast<int>(std::round(min + normalized * (max - min))), min, max);

        if (newCount != unisonVoices)
        {
            unisonVoices = newCount;
            updateUnisonOffsets();
            updateFrequencies();
        }
        return;
    }

    if (id == ParamIDs::UNISON_DETUNE.data())
    {
        unisonDetune = static_cast<double>(normalized) * 100.0;
        updateUnisonOffsets();
        updateFrequencies();
        return;
    }

    if (id == ParamIDs::UNISON_SPREAD.data())
    {
        unisonSpread = static_cast<double>(normalized);
        updateUnisonOffsets();
        return;
    }

    // --- 2. VOICE GLIDE & BEND PARAMETERS ---
    if (id == ParamIDs::VOICE_GLIDE.data())
    {
        glideTimeMs = static_cast<double>(normalized) * 2000.0;
        return;
    }

    if (id == ParamIDs::VOICE_BEND.data())
    {
        pitchBendRange = (normalized <= 1.0f) ? static_cast<double>(normalized) * 24.0 : static_cast<double>(normalized);
        return;
    }
    
    if (id == ParamIDs::VOICE_VELOCITY.data())
    {
        velocityDepth = static_cast<double>(normalized);
        return;
    }

    if (id == ParamIDs::VOICE_MOD_CUTOFF.data())
    {
        // Convert normalized [0.0, 1.0] to bipolar [-1.0, 1.0]
        modWheelCutoffDepth = (static_cast<double>(normalized) - 0.5) * 2.0;
        return;
    }
        
    if (id == ParamIDs::VOICE_DRIFT.data())
    {   
        voiceDrift = static_cast<double>(normalized);
        return;
    }

    // 3. FM Matrix Parameters
    if (id == ParamIDs::FM_ACTIVE.data() ||
        id == ParamIDs::FM_OSC3_RATIO.data()||
        id == ParamIDs::FM_DEPTH_3TO2.data() ||
        id == ParamIDs::FM_OSC3_FEEDBACK.data() ||
        id == ParamIDs::FM_OSC2_RATIO.data() ||
        id == ParamIDs::FM_DEPTH_2TO1.data() ||
        id == ParamIDs::FM_MASTER_AMOUNT.data())
    {
        fmProcessor.setParam(id, normalized);
        if (id == ParamIDs::FM_OSC2_RATIO.data() || id == ParamIDs::FM_OSC3_RATIO.data())
        {
            updateFrequencies();
        }
        return;
    }

    // 3. Oscillator parameters & Routings
    for (int i = 1; i <= 3; ++i)
    {
        int idx = i - 1;
        
        if (id == "osc" + juce::String(i) + "_shape")
        {
            baseOscParams[idx].baseShape = normalized;
            baseOscParams[idx].shape = normalized;
            updateOscillatorParams(idx);
            return;
        }
        if (id == "osc" + juce::String(i) + "_rolloff")
        {
            baseOscParams[idx].rolloff = normalized;
            updateOscillatorParams(idx);
            return;
        }
        if (id == "osc" + juce::String(i) + "_octave")
        {
            baseOscParams[idx].octave = static_cast<int>(std::round(-2.0 + static_cast<double>(normalized) * 4.0));
            updateFrequencies();
            return;
        }
        if (id == "osc" + juce::String(i) + "_semitone")
        {
            baseOscParams[idx].semitone = (normalized <= 1.0f) ? static_cast<int>(std::round(-12 + normalized * 24)) : static_cast<int>(normalized);
            updateFrequencies();
            return;
        }
        if (id == "osc" + juce::String(i) + "_fine")
        {
            baseOscParams[idx].fine = (normalized <= 1.0f) ? (-50.0 + static_cast<double>(normalized) * 100.0) : static_cast<double>(normalized);
            updateFrequencies();
            return;
        }
        if (id == "osc" + juce::String(i) + "_pan")
        {
            baseOscParams[idx].pan = (normalized <= 1.0f) ? -1.0 + normalized * 2.0 : static_cast<double>(normalized);
            return;
        }
        if (id == "osc" + juce::String(i) + "_level")
        {
            baseOscParams[idx].level = normalized;
            return;
        }

        if (id == "filter_env_osc" + juce::String(i)) { filterEnvToOsc[idx] = normalized; return; }
        if (id == "pitch_env_osc" + juce::String(i))  { pitchEnvToOsc[idx] = normalized; return; }
        if (id == "mod_env_osc" + juce::String(i))    { modEnvToOsc[idx] = normalized; return; }
    }

    // 4. Filter Parameters
    if (id == ParamIDs::FILTER_TYPE.data()) {
        double roundedType = std::round(normalized <= 1.0f ? static_cast<double>(normalized) * 2.0 : static_cast<double>(normalized));
        filterParams.type = static_cast<int>(roundedType);
    }
    if (id == ParamIDs::FILTER_CUTOFF.data()) {
        filterParams.cutoff = 20.0 + static_cast<double>(normalized) * 20000.0;
    }
    if (id == ParamIDs::FILTER_RESONANCE.data())  filterParams.resonance = normalized;
    if (id == ParamIDs::FILTER_DRIVE.data())      filterParams.drive = normalized;
    if (id == ParamIDs::FILTER_FEEDBACK.data())   filterParams.feedback = normalized;
    if (id == ParamIDs::FILTER_MORPH.data())      filterParams.morph = normalized;
    if (id == ParamIDs::FILTER_ENV_AMOUNT.data()) filterParams.envAmount = (normalized <= 1.0f) ? -1.0 + static_cast<double>(normalized) * 2.0 : normalized;

    // 5. Sync Toggles
    if (id == ParamIDs::OSC2_SYNC_TO_OSC1.data()) syncToOsc2 = normalized > 0.5f;
    if (id == ParamIDs::OSC3_SYNC_TO_OSC1.data()) syncToOsc3 = normalized > 0.5f;

    // 6. Modulation Routing & Rolloff Parameters
    if (id == ParamIDs::MOD_ENV_SHAPE1.data()) { modEnvToShape1 = normalized > 0.5f ? 1.0 : 0.0; return; }
    if (id == ParamIDs::MOD_ENV_SHAPE2.data()) { modEnvToShape2 = normalized > 0.5f ? 1.0 : 0.0; return; }
    if (id == ParamIDs::MOD_ENV_SHAPE3.data()) { modEnvToShape3 = normalized > 0.5f ? 1.0 : 0.0; return; }
    if (id == ParamIDs::MOD_ENV_FM.data())     { modEnvToFm = normalized > 0.5f ? 1.0 : 0.0; return; }

    // 7. Pitch Envelope Amount (Bipolar)
    if (id == ParamIDs::PITCH_ENV_AMOUNT.data())
    {
        pitchEnvAmount = static_cast<double>(normalized);
        return;
    }

    // 8. Envelope Parameter Mappings
    auto mapEnvelope = [normalized, &id](ADSREnvelope& env, const juce::String& prefix) {
        ADSREnvelope::Parameters params;
        double val = static_cast<double>(normalized);
        double scaledTime = normalized <= 1.0f ? 0.001 + static_cast<double>(normalized) * 9.999 : val;
        double scaledSlope = normalized <= 1.0f ? -20.0 + static_cast<double>(normalized) * 40.0 : val;

        if (id == prefix + "_attack")         { params.attack = scaledTime; }
        else if (id == prefix + "_attack_slope")  { params.attackSlope = scaledSlope; }
        else if (id == prefix + "_decay")          { params.decay = scaledTime; }
        else if (id == prefix + "_decay_slope")   { params.decaySlope = scaledSlope; }
        else if (id == prefix + "_sustain")         { params.sustain = val; }
        else if (id == prefix + "_release")         { params.release = scaledTime; }
        else if (id == prefix + "_release_slope") { params.releaseSlope = scaledSlope; }
        else { return; }

        env.setParameters(params);
    };

    mapEnvelope(ampEnv, "amp_env");
    mapEnvelope(filterEnv, "filter_env");
    mapEnvelope(pitchEnv, "pitch_env");
    mapEnvelope(modEnv, "mod_env");

    // 9. LFO Parameters
    if (id == ParamIDs::LFO1_RATE.data())
    {
        lfo1.setRate(20.0 * static_cast<double>(normalized));
        return;
    }
    if (id == ParamIDs::LFO1_DEPTH.data())
    {
        lfo1Depth = static_cast<double>(normalized);
        return;
    }
    if (id == ParamIDs::LFO1_SHAPE.data())
    {
        lfo1.setShape(static_cast<double>(normalized));
        return;
    }
    if (id == ParamIDs::LFO1_DEST_OSC1_PITCH.data())
    {
        lfo1DestOsc1Pitch = normalized > 0.5f;
        return;
    }
    if (id == ParamIDs::LFO1_DEST_OSC1_SHAPE.data())
    {
        lfo1DestOsc1Shape = normalized > 0.5f;
        return;
    }
    if (id == ParamIDs::LFO1_DEST_CUTOFF.data())
    {
        lfo1DestCutoff = normalized > 0.5f;
        return;
    }

    // LFO 2 Parameters
    if (id == ParamIDs::LFO2_RATE.data())
    {
        lfo2.setRate(20.0 * static_cast<double>(normalized));
        return;
    }
    if (id == ParamIDs::LFO2_DEPTH.data())
    {
        lfo2Depth = static_cast<double>(normalized);
        return;
    }
    if (id == ParamIDs::LFO2_SHAPE.data())
    {
        lfo2.setShape(static_cast<double>(normalized));
        return;
    }
    if (id == ParamIDs::LFO2_DEST_OSC2_PITCH.data())
    {
        lfo2DestOsc2Pitch = normalized > 0.5f;
        return;
    }
    if (id == ParamIDs::LFO2_DEST_OSC2_SHAPE.data())
    {
        lfo2DestOsc2Shape = normalized > 0.5f;
        return;
    }
    if (id == ParamIDs::LFO2_DEST_FM_AMOUNT.data())
    {
        lfo2DestFMAmount = normalized > 0.5f;
        return;
    }

    // 10. Voice Parameters
    if (id == ParamIDs::VOICE_GLIDE.data())
    {
        glideTimeMs = static_cast<double>(normalized) * 2000.0;
        return;
    }

    if (id == ParamIDs::VOICE_BEND.data())
    {
        pitchBendRange = (normalized <= 1.0f) ? static_cast<double>(normalized) * 24.0 : static_cast<double>(normalized);
        return;
    }

    if (id == ParamIDs::TUBE_DRIVE.data())
    {
        double driveVal = normalized;
        tubeDriveLeft.setDrive(driveVal);
        tubeDriveRight.setDrive(driveVal);
        return;
    }
}

void SynthVoice::processBlock(double* outputBufferLeft, double* outputBufferRight, int numSamples)
{
    if (!active) return;

    int activeCount = std::clamp(unisonVoices, 1, maxUnisonVoices);
    double unisonHeadroom = 1.0 / std::pow(static_cast<double>(activeCount), 0.65);

    for (int i = 0; i < numSamples; ++i)
    {
        double ampVal = ampEnv.processSample();
        double filterVal = filterEnv.processSample();
        double modVal = modEnv.processSample();
        double pitchVal = pitchEnv.processSample();

        // ======================================================================
        // CRITICAL FIX: Stop processing and mark voice as inactive when done
        // ======================================================================
        if (ampEnv.getState() == ADSREnvelope::State::Idle) 
        { 
            active = false; // SynthEngine will now skip this voice, freeing it up
            break; 
        }

        // --- MOD ENVELOPE SHAPE MODULATION FLAGS ---
        bool modShapeActive1 = (modEnvToShape1 > 0.5);
        bool modShapeActive2 = (modEnvToShape2 > 0.5);
        bool modShapeActive3 = (modEnvToShape3 > 0.5);

        // --- LFO 1 PROCESSING (Computed Once per Sample) ---
        double lfo1Sample = lfo1.processSample();
        double lfo1Val = lfo1Sample * lfo1Depth;

        bool lfo1HasDestinations = lfo1DestOsc1Pitch || lfo1DestOsc1Shape || lfo1DestCutoff;
        double lfo1AmpMod = 1.0;

        double osc1PitchMod = 0.0;
        double osc1ShapeModVal = -1.0;
        double lfo1CutoffMod = 0.0;

        if (!lfo1HasDestinations)
        {
            lfo1AmpMod = 1.0 + (lfo1Val * 0.5); // Fallback tremolo
        }
        else
        {
            if (lfo1DestOsc1Pitch)  { osc1PitchMod = lfo1Val * 2.0; }
            if (lfo1DestOsc1Shape)  { osc1ShapeModVal = std::clamp(baseOscParams[0].baseShape + (lfo1Val * 0.5), 0.0, 1.0); }
            if (lfo1DestCutoff)     { lfo1CutoffMod = lfo1Val * 2000.0; }
        }

        // --- LFO 2 PROCESSING (Computed Once per Sample) ---
        double lfo2Sample = lfo2.processSample();
        double lfo2Val = lfo2Sample * lfo2Depth;

        bool lfo2HasDestinations = lfo2DestOsc2Pitch || lfo2DestOsc2Shape || lfo2DestFMAmount;
        double lfo2AmpMod = 1.0;

        double osc2PitchMod = 0.0;
        double osc2ShapeModVal = -1.0;
        double lfo2FMAmountMod = 0.0;

        if (!lfo2HasDestinations)
        {
            lfo2AmpMod = 1.0 + (lfo2Val * 0.5);
        }
        else
        {
            if (lfo2DestOsc2Pitch)  { osc2PitchMod = lfo2Val * 2.0; }
            if (lfo2DestOsc2Shape)  { osc2ShapeModVal = std::clamp(baseOscParams[1].baseShape + (lfo2Val * 0.5), 0.0, 1.0); }
            if (lfo2DestFMAmount)   { lfo2FMAmountMod = lfo2Val * 0.5; }
        }

        // Calculate bipolar pitch envelope depth in semitones
        double bipolarAmount = (pitchEnvAmount - 0.5) * 2.0;
        double totalPitchEnvSemis = pitchVal * bipolarAmount * 12.0;

        // --- PORTAMENTO, PITCH BEND & PITCH ENVELOPE MODULATION ---
        if (targetNote >= 0.0)
        {
            if (glideTimeMs > 0.0)
            {
                double timeSeconds = glideTimeMs * 0.001;
                double glideCoeff = 1.0 - std::exp(-1.0 / (timeSeconds * sampleRate));
                
                currentNote += (targetNote - currentNote) * glideCoeff;
                if (std::abs(targetNote - currentNote) < 0.001)
                {
                    currentNote = targetNote;
                }
            }
            else
            {
                currentNote = targetNote;
            }

            double bendSemis = pitchBendVal * pitchBendRange;

            for (int u = 0; u < activeCount; ++u)
            {
                auto& layer = unisonLayers[u];
                double effectiveNote = currentNote + (layer.cents / 100.0) + bendSemis;
                double baseFreq = 440.0 * std::pow(2.0, (effectiveNote - 69.0) / 12.0);

                for (int oscIndex = 0; oscIndex < 3; ++oscIndex)
                {
                    const auto& p = baseOscParams[oscIndex];
                    double semitoneTotal = static_cast<double>(p.semitone) + (p.fine / 100.0);
                    
                    double ratio = 1.0;
                    if (oscIndex == 1) ratio = fmProcessor.getOsc2Ratio();
                    else if (oscIndex == 2) ratio = fmProcessor.getOsc3Ratio();

                    double oscEnvSemis = totalPitchEnvSemis * pitchEnvToOsc[oscIndex];
                    
                    // Inject LFO pitch modulations per oscillator
                    double lfoPitchOffset = 0.0;
                    if (oscIndex == 0 && lfo1HasDestinations && lfo1DestOsc1Pitch) lfoPitchOffset += osc1PitchMod;
                    if (oscIndex == 1 && lfo2HasDestinations && lfo2DestOsc2Pitch) lfoPitchOffset += osc2PitchMod;

                    double freq = baseFreq * ratio * std::pow(2.0, static_cast<double>(p.octave) + ((semitoneTotal + oscEnvSemis + lfoPitchOffset) / 12.0));
                    
                    layer.oscillators[oscIndex].setPitch(freq);
                }
            }
        }

        // Modulate Oscillator Shapes via Mod Envelope Checkboxes
        for (int oscIndex = 0; oscIndex < 3; ++oscIndex)
        {
            auto& p = baseOscParams[oscIndex];
            
            bool isModActive = (oscIndex == 0 && modShapeActive1) ||
                               (oscIndex == 1 && modShapeActive2) ||
                               (oscIndex == 2 && modShapeActive3);

            // Default to the user's base shape when checkbox is off
            double targetShape = p.baseShape; 

            if (isModActive)
            {
                // Modulate shape only when active
                targetShape = std::clamp(p.baseShape * modVal, 0.0, 1.0);
            }
            
            for (int u = 0; u < activeCount; ++u)
            {
                unisonLayers[u].oscillators[oscIndex].setModulatedShape(targetShape);
            }
        }

        double mixLeft = 0.0;
        double mixRight = 0.0;
        double bypassLeft = 0.0;
        double bypassRight = 0.0;

        for (int u = 0; u < activeCount; ++u)
        {
            auto& layer = unisonLayers[u];
            auto& oscs = layer.oscillators;

            // Apply LFO shape modulation efficiently on primary layer (or all layers if desired)
            if (lfo1HasDestinations && lfo1DestOsc1Shape && osc1ShapeModVal >= 0.0)
            {
                oscs[0].setModulatedShape(osc1ShapeModVal);
            }
            if (lfo2HasDestinations && lfo2DestOsc2Shape && osc2ShapeModVal >= 0.0)
            {
                oscs[1].setModulatedShape(osc2ShapeModVal);
            }

            double sig1 = 0.0, sig2 = 0.0, sig3 = 0.0;

            double masterAmount = fmProcessor.getMasterAmount() + lfo2FMAmountMod;
            masterAmount = std::clamp(masterAmount, 0.0, 1.0);
            bool fmActive = fmProcessor.isActive();

            if (fmActive)
            {
                double fmEnvMod = (modEnvToFm > 0.5) ? modVal : 1.0; // Ensure 0.0 fallback when off
                double modValFast = std::pow(fmEnvMod, 0.25);
                double modValSlow = std::pow(fmEnvMod, 0.8);
                double fastAmount = masterAmount * modValFast;
                double slowAmount = masterAmount * modValSlow;

                double feedback3 = fmProcessor.getOsc3Feedback();
                double depth32 = fmProcessor.getDepth3to2();
                double depth21 = fmProcessor.getDepth2to1();

                // 1. Oscillator 3 (Using layer-specific feedback history)
                double mod3FB = layer.lastOsc3Sample * feedback3;
                double rawSig3 = oscs[2].processSample(mod3FB);
                layer.lastOsc3Sample = rawSig3; // Store back to this layer only

                // 2. Oscillator 2
                double mod3to2 = rawSig3 * depth32 * fastAmount;
                double rawSig2 = oscs[1].processSample(mod3to2);

                // 3. Oscillator 1
                double mod2to1 = rawSig2 * depth21 * slowAmount;
                sig1 = oscs[0].processSample(mod2to1) * baseOscParams[0].level;

                sig2 = rawSig2 * baseOscParams[1].level;
                sig3 = rawSig3 * baseOscParams[2].level;
            }
            else
            {
                sig1 = oscs[0].processSample() * baseOscParams[0].level;
                bool osc1Wrapped = oscs[0].isWrapped();

                if (syncToOsc2 && osc1Wrapped) oscs[1].resetPhase();
                sig2 = oscs[1].processSample() * baseOscParams[1].level;

                if (syncToOsc3 && osc1Wrapped) oscs[2].resetPhase();
                sig3 = oscs[2].processSample() * baseOscParams[2].level;
            }

            double signals[3] = { sig1, sig2, sig3 };
            for (int oscIndex = 0; oscIndex < 3; ++oscIndex)
            {
                double oscSample = signals[oscIndex];
                if (oscSample == 0.0) continue;

                double basePan = baseOscParams[oscIndex].pan;
                double totalPan = std::clamp(basePan + layer.panOffset, -1.0, 1.0);
                double panAngle = (totalPan + 1.0) * 0.25 * juce::MathConstants<double>::pi;

                double gainLeft = std::cos(panAngle);
                double gainRight = std::sin(panAngle);

                // Check if this oscillator is routed to the filter (threshold > 0.5 for toggle, or used as amount)
                bool routeToFilter = filterEnvToOsc[oscIndex] > 0.5;

                if (routeToFilter)
                {
                    mixLeft += oscSample * gainLeft * unisonHeadroom;
                    mixRight += oscSample * gainRight * unisonHeadroom;
                }
                else
                {
                    // Bypasses the filter completely
                    bypassLeft += oscSample * gainLeft * unisonHeadroom;
                    bypassRight += oscSample * gainRight * unisonHeadroom;
                }
            }
        }

        mixLeft *= 6.0;
        mixRight *= 6.0;
        bypassLeft *= 6.0;
        bypassRight *= 6.0;

        // Filter Envelope & LFO Cutoff Modulation Mapping
        double maxFreq = 20000.0;
        double minFreq = 20.0;
        double baseNorm = std::log(std::max(minFreq, filterParams.cutoff) / minFreq) / std::log(maxFreq / minFreq);
        double maxNormSpan = 1.0 - baseNorm;
        double modulatedNorm = baseNorm + (filterVal * filterParams.envAmount * maxNormSpan);

        // --- MOD WHEEL CUTOFF MODULATION ---
        // modWheelVal is 0.0 to 1.0. modWheelCutoffDepth is -1.0 to 1.0.
        double modWheelEffect = modWheelVal * modWheelCutoffDepth;
        if (modWheelEffect > 0.0)
        {
            modulatedNorm += modWheelEffect * (1.0 - modulatedNorm); // Scales towards max
        }
        else if (modWheelEffect < 0.0)
        {
            modulatedNorm += modWheelEffect * modulatedNorm; // Scales towards min
        }
        // -----------------------------------

     //   double expVal = (filterParams.type == 1) ? 1.6 : ((filterParams.type == 2) ? 1.45 : 1.55);
        double expVal = (filterParams.type == 1) ? 1.65 : ((filterParams.type == 2) ? 1.6 : 2.0);
        double mod = std::pow(std::clamp(modulatedNorm, 0.0, 1.0), expVal);
        double cutoffMod = minFreq * std::pow(1000.0, mod);

        if (lfo1HasDestinations && lfo1DestCutoff)
        {
            cutoffMod += lfo1CutoffMod;
        }
        cutoffMod = std::clamp(cutoffMod, minFreq, maxFreq);

        // Process through Left and Right Stereo Filters
        double filteredLeft = filterLeft.process(mixLeft, filterParams.type, cutoffMod, filterParams.resonance, filterParams.drive, filterParams.feedback, filterParams.morph);
        double filteredRight = filterRight.process(mixRight, filterParams.type, cutoffMod, filterParams.resonance, filterParams.drive, filterParams.feedback, filterParams.morph);

        // Combine filtered audio and filter-bypassed audio
        double summedLeft = filteredLeft + bypassLeft;
        double summedRight = filteredRight + bypassRight;

        // Combine amplitude envelope, velocity, and LFO fallback tremolo modulations
        double totalGain = ampVal * voiceVelocityGain * lfo1AmpMod * lfo2AmpMod;

        double voiceOutLeft = summedLeft * totalGain * 0.7;
        double voiceOutRight = summedRight * totalGain * 0.7;

        // --- TUBE DRIVE SATURATION (End of Processing Chain) ---
        voiceOutLeft = tubeDriveLeft.processSample(voiceOutLeft);
        voiceOutRight = tubeDriveRight.processSample(voiceOutRight);

        outputBufferLeft[i] += voiceOutLeft;
        if (outputBufferRight != nullptr)
        {
            outputBufferRight[i] += voiceOutRight;
        }
    }
}