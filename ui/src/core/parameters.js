// src/core/parameters.js
import { PARAM_IDS } from './paramIds.js';
import { presetManager } from './PresetManager.js'

export function registerAllParameters(paramStore) {
    paramStore.register({
        id: PARAM_IDS.MASTER_VOLUME,
        name: 'Master Volume',
        defaultValue: 0.7,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.TUBE_DRIVE,
        name: 'Tube Drive',
        defaultValue: 0.0,
        min: 0,
        max: 1
    });

    // Register parameters for Osc 1, Osc 2, and Osc 3 in a loop
    [1, 2, 3].forEach((oscNr) => {
        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_SHAPE`],
            name: `Osc ${oscNr} Shape`,
            defaultValue: 1.0, // 0.5 normalized
            min: 0,
            max: 1
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_OCTAVE`],
            name: `Osc ${oscNr} Octave`,
            defaultValue: 0, // Real value: 0 octaves (Center = 0.5 normalized)
            min: -2,
            max: 2,
            steps: 5
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_SEMITONE`],
            name: `Osc ${oscNr} Semitone`,
            defaultValue: 0, // Real value: 0 semitones (Center = 0.5 normalized)
            min: -12,
            max: 12,
            steps: 25
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_FINE`],
            name: `Osc ${oscNr} Fine`,
            defaultValue: 0, // Real value: 0 cents (Center = 0.5 normalized)
            min: -50,
            max: 50
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_ROLLOFF`],
            name: `Osc ${oscNr} Roll-off`,
            defaultValue: 0.98,
            min: 0,
            max: 1
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_PAN`],
            name: `Osc ${oscNr} Pan`,
            defaultValue: 0, // Real value: 0 Center (Center = 0.5 normalized)
            min: -1,
            max: 1
        });

        paramStore.register({
            id: PARAM_IDS[`OSC${oscNr}_LEVEL`],
            name: `Osc ${oscNr} Level`,
            defaultValue: oscNr == 1 ? 0.7 : 0,
            min: 0,
            max: 1
        });
    });

    // Sync Toggles
    paramStore.register({
        id: PARAM_IDS.OSC2_SYNC_TO_OSC1,
        name: 'Sync Osc 2 to Osc 1',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.OSC3_SYNC_TO_OSC1,
        name: 'Sync Osc 3 to Osc 1',
        defaultValue: 0,
        min: 0,
        max: 1
    });

// --- FM Matrix Parameters ---
    paramStore.register({
        id: PARAM_IDS.FM_ACTIVE,
        name: 'FM Active',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FM_OSC3_RATIO,
        name: 'Osc 3 FM Ratio',
        defaultValue: 1.0, // 1.0 maps to index 1 ('1')
        min: 0,
        max: 12,
        steps: 13
    });

    paramStore.register({
        id: PARAM_IDS.FM_DEPTH_3TO2,
        name: 'FM Depth 3 -> 2',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FM_OSC3_FEEDBACK,
        name: 'Osc 3 Feedback',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FM_OSC2_RATIO,
        name: 'Osc 2 FM Ratio',
        defaultValue: 1.0,
        min: 0,
        max: 12,
        steps: 13
    });

    paramStore.register({
        id: PARAM_IDS.FM_DEPTH_2TO1,
        name: 'FM Depth 2 -> 1',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FM_MASTER_AMOUNT,
        name: 'Master FM Amount',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    // --- LFO 1 Parameters ---
    paramStore.register({
        id: PARAM_IDS.LFO1_RATE,
        name: 'LFO 1 Rate',
        defaultValue: 1.0, // Hz
        min: 0.1,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.LFO1_DEPTH,
        name: 'LFO 1 Depth',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.LFO1_SHAPE,
        name: 'LFO 1 Shape',
        defaultValue: 0, // 0 = Sine, 1 = Triangle
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO1_DEST_OSC1_PITCH,
        name: 'LFO 1 -> Osc 1 Pitch',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO1_DEST_OSC1_SHAPE,
        name: 'LFO 1 -> Osc 1 Shape',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO1_DEST_CUTOFF,
        name: 'LFO 1 -> Filter Cutoff',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    // --- LFO 2 Parameters ---
    paramStore.register({
        id: PARAM_IDS.LFO2_RATE,
        name: 'LFO 2 Rate',
        defaultValue: 1.0,
        min: 0.1,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.LFO2_DEPTH,
        name: 'LFO 2 Depth',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.LFO2_SHAPE,
        name: 'LFO 2 Shape',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO2_DEST_OSC2_PITCH,
        name: 'LFO 2 -> Osc 2 Pitch',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO2_DEST_OSC2_SHAPE,
        name: 'LFO 2 -> Osc 2 Shape',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    paramStore.register({
        id: PARAM_IDS.LFO2_DEST_FM_AMOUNT,
        name: 'LFO 2 -> FM Master Amount',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: 2
    });

    // --- Multimode Non-Linear Filter Parameters ---
    paramStore.register({
        id: PARAM_IDS.FILTER_TYPE,
        name: 'Filter Type',
        defaultValue: 0, // 0 = 18dB ZDF, 1 = 24dB Ladder, 2 = 18dB Diode
        min: 0,
        max: 2,
        steps: 3
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_CUTOFF,
        name: 'Filter Cutoff',
        defaultValue: 20000,
        min: 20,
        max: 20000
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_RESONANCE,
        name: 'Filter Resonance',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_DRIVE,
        name: 'Filter Drive',
        defaultValue: 0.0, // 0 = Clean, 1 = Hard Drive
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_FEEDBACK,
        name: 'Filter Feedback',
        defaultValue: 0.0, // Extra resonance loop saturation/drive
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_MORPH,
        name: 'Filter Morph',
        defaultValue: 0.0, // 0.0 = Low-Pass, 0.5 = Band-Pass, 1.0 = High-Pass
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_AMOUNT,
        name: 'Filter Env Depth',
        defaultValue: 0.0,
        min: -1.0,
        max: 1.0
    });

    // --- Unison & Voice Parameters ---
    paramStore.register({
        id: PARAM_IDS.VOICE_MODE,
        name: 'Voice Mode',
        defaultValue: 0, // 0 = Poly, 1 = Mono, 2 = Legato
        min: 0,
        max: 2,
        steps: 3
    });

    paramStore.register({
        id: PARAM_IDS.UNISON_VOICES,
        name: 'Unison Voices',
        defaultValue: 1,
        min: 1,
        max: 8,
        steps: 8
    });

    paramStore.register({
        id: PARAM_IDS.UNISON_DETUNE,
        name: 'Unison Detune',
        defaultValue: 15, // Cents
        min: 0,
        max: 100
    });

    paramStore.register({
        id: PARAM_IDS.UNISON_SPREAD,
        name: 'Unison Stereo Spread',
        defaultValue: 0.8, // 80% stereo width
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.VOICE_DRIFT,
        name: 'Analog Pitch Drift',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.VOICE_GLIDE,
        name: 'Portamento Glide',
        defaultValue: 0,
        min: 0,
        max: 200
    });

    paramStore.register({
        id: PARAM_IDS.VOICE_BEND,
        name: 'Pitch Bend Range',
        defaultValue: 2, // Semitones (+/- 2 st default)
        min: 0,
        max: 24,
        steps: 25
    });

    paramStore.register({
        id: PARAM_IDS.VOICE_VELOCITY,
        name: 'Velocity Depth',
        defaultValue: 0.5,
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.VOICE_MOD_CUTOFF,
        name: 'Mod Wheel Cutoff',
        defaultValue: 0.5,
        min: -1.0,
        max: 1.0
    });

    // --- Amp Envelope Parameters ---
    paramStore.register({
        id: PARAM_IDS.AMP_ENV_ATTACK,
        name: 'Amp Env Attack',
        defaultValue: 0.005, // Seconds (5ms)
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_ATTACK_SLOPE,
        name: 'Amp Env Attack Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_DECAY,
        name: 'Amp Env Decay',
        defaultValue: 0.2,
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_DECAY_SLOPE,
        name: 'Amp Env Decay Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_SUSTAIN,
        name: 'Amp Env Sustain',
        defaultValue: 0.7, // Level (0.0 to 1.0)
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_RELEASE,
        name: 'Amp Env Release',
        defaultValue: 0.25, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.AMP_ENV_RELEASE_SLOPE,
        name: 'Amp Env Release Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    // --- Filter Envelope Parameters ---
    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_ATTACK,
        name: 'Filter Env Attack',
        defaultValue: 0.01, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_ATTACK_SLOPE,
        name: 'Filter Env Attack Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_DECAY,
        name: 'Filter Env Decay',
        defaultValue: 0.3, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_DECAY_SLOPE,
        name: 'Filter Env Decay Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_SUSTAIN,
        name: 'Filter Env Sustain',
        defaultValue: 0.5, // Level (0.0 to 1.0)
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_RELEASE,
        name: 'Filter Env Release',
        defaultValue: 0.25, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_RELEASE_SLOPE,
        name: 'Filter Env Release Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_OSC1,
        name: 'Filter Env to Osc 1',
        defaultValue: 1,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_OSC2,
        name: 'Filter Env to Osc 2',
        defaultValue: 1,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.FILTER_ENV_OSC3,
        name: 'Filter Env to Osc 3',
        defaultValue: 1,
        min: 0,
        max: 1
    });

    // --- Pitch Envelope Parameters ---
    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_ATTACK,
        name: 'Pitch Env Attack',
        defaultValue: 0.01, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_ATTACK_SLOPE,
        name: 'Pitch Env Attack Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_DECAY,
        name: 'Pitch Env Decay',
        defaultValue: 0.3, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_DECAY_SLOPE,
        name: 'Pitch Env Decay Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_SUSTAIN,
        name: 'Pitch Env Sustain',
        defaultValue: 0.5, // Level (0.0 to 1.0)
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_RELEASE,
        name: 'Pitch Env Release',
        defaultValue: 0.25, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_RELEASE_SLOPE,
        name: 'Pitch Env Release Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_OSC1,
        name: 'Pitch to Osc 1',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_OSC2,
        name: 'Pitch Env to Osc 2',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.PITCH_ENV_OSC3,
        name: 'Pitch Env to Osc 3',
        defaultValue: 0,
        min: 0,
        max: 1
    });

        paramStore.register({
        id: PARAM_IDS.PITCH_ENV_AMOUNT,
        name: 'Pitch Env Amount',
        defaultValue: 0.0,
        min: 0.0,
        max: 1.0
    });

    // --- Mod Envelope Parameters ---
    paramStore.register({
        id: PARAM_IDS.MOD_ENV_ATTACK,
        name: 'Mod Env Attack',
        defaultValue: 0.01, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_ATTACK_SLOPE,
        name: 'Mod Env Attack Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_DECAY,
        name: 'Mod Env Decay',
        defaultValue: 0.3, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_DECAY_SLOPE,
        name: 'Mod Env Decay Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_SUSTAIN,
        name: 'Mod Env Sustain',
        defaultValue: 0.5, // Level (0.0 to 1.0)
        min: 0.0,
        max: 1.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_RELEASE,
        name: 'Mod Env Release',
        defaultValue: 0.25, // Seconds
        min: 0.001,
        max: 10.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_RELEASE_SLOPE,
        name: 'Mod Env Release Slope',
        defaultValue: 3.0,
        min: -20.0,
        max: 20.0
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_SHAPE1,
        name: 'Mod Env to Osc 1 Shape',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_SHAPE2,
        name: 'Mod Env to Osc 2 Shape',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_SHAPE3,
        name: 'Mod Env to Osc 3 Shape',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    paramStore.register({
        id: PARAM_IDS.MOD_ENV_FM,
        name: 'Mod Env to FM Amount',
        defaultValue: 0,
        min: 0,
        max: 1
    });

    // --- Preset Selector Parameters ---

    const factoryNames = presetManager.getFactoryNames();

    paramStore.register({
        id: 'preset_index',
        name: 'Preset Index',
        defaultValue: 0,
        min: 0,
        max: 1,
        steps: factoryNames.length
    });
}