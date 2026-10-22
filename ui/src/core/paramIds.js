// src/core/paramIds.js
export const PARAM_IDS = Object.freeze({

    // Master volume
    MASTER_VOLUME: 'master_volume',

    // Tube drive
    TUBE_DRIVE: 'tube_drive',

    // Oscillator 1
    OSC1_SHAPE: 'osc1_shape',
    OSC1_OCTAVE: 'osc1_octave',
    OSC1_SEMITONE: 'osc1_semitone',
    OSC1_FINE: 'osc1_fine',
    OSC1_ROLLOFF: 'osc1_rolloff',
    OSC1_PAN: 'osc1_pan',
    OSC1_LEVEL: 'osc1_level',

    // Oscillator 2
    OSC2_SHAPE: 'osc2_shape',
    OSC2_OCTAVE: 'osc2_octave',
    OSC2_SEMITONE: 'osc2_semitone',
    OSC2_FINE: 'osc2_fine',
    OSC2_ROLLOFF: 'osc2_rolloff',
    OSC2_PAN: 'osc2_pan',
    OSC2_LEVEL: 'osc2_level',

    // Oscillator 3
    OSC3_SHAPE: 'osc3_shape',
    OSC3_OCTAVE: 'osc3_octave',
    OSC3_SEMITONE: 'osc3_semitone',
    OSC3_FINE: 'osc3_fine',
    OSC3_ROLLOFF: 'osc3_rolloff',
    OSC3_PAN: 'osc3_pan',
    OSC3_LEVEL: 'osc3_level',

    // Sync Toggles
    OSC2_SYNC_TO_OSC1: 'osc2_sync_to_osc1',
    OSC3_SYNC_TO_OSC1: 'osc3_sync_to_osc1',

    // FM Matrix IDs
    FM_ACTIVE: 'fm_active',
    FM_OSC3_RATIO: 'fm_osc3_ratio',
    FM_DEPTH_3TO2: 'fm_depth_3to2',
    FM_OSC3_FEEDBACK: 'fm_osc3_feedback',
    FM_OSC2_RATIO: 'fm_osc2_ratio',
    FM_DEPTH_2TO1: 'fm_depth_2to1',
    FM_MASTER_AMOUNT: 'fm_master_amount',

    // LFO 1
    LFO1_RATE:            'lfo1_rate',
    LFO1_DEPTH:           'lfo1_depth',
    LFO1_SHAPE:           'lfo1_shape',           // 0: Sine, 1: Triangle
    LFO1_DEST_OSC1_PITCH: 'lfo1_dest_osc1_pitch', // 0: Off, 1: On
    LFO1_DEST_OSC1_SHAPE: 'lfo1_dest_osc1_shape', // 0: Off, 1: On
    LFO1_DEST_CUTOFF:     'lfo1_dest_cutoff',     // 0: Off, 1: On

    // LFO 2
    LFO2_RATE:            'lfo2_rate',
    LFO2_DEPTH:           'lfo2_depth',
    LFO2_SHAPE:           'lfo2_shape',           // 0: Sine, 1: Triangle
    LFO2_DEST_OSC2_PITCH: 'lfo2_dest_osc2_pitch', // 0: Off, 1: On
    LFO2_DEST_OSC2_SHAPE: 'lfo2_dest_osc2_shape', // 0: Off, 1: On
    LFO2_DEST_FM_AMOUNT:  'lfo2_dest_fm_amount',   // 0: Off, 1: On

    // Filter IDs
    FILTER_TYPE: 'filter_type',             // 0 = 18dB ZDF, 1 = 24dB Ladder, 2 = 18dB Diode
    FILTER_CUTOFF: 'filter_cutoff',
    FILTER_RESONANCE: 'filter_resonance',
    FILTER_DRIVE: 'filter_drive',
    FILTER_FEEDBACK: 'filter_feedback',
    FILTER_MORPH: 'filter_morph',
    FILTER_ENV_AMOUNT: 'filter_env_amount',

    // Unison & Voice Character
    UNISON_VOICES:    'unison_voices',
    UNISON_DETUNE:    'unison_detune',
    UNISON_SPREAD:    'unison_spread',

    // Performance & Expressivity
    VOICE_GLIDE:      'voice_glide',
    VOICE_BEND:       'voice_bend',
    VOICE_VELOCITY:   'voice_velocity',
    VOICE_MOD_CUTOFF: 'voice_mod_cutoff',
    VOICE_DRIFT:      'voice_drift',

    // Voice Mode State
    VOICE_MODE:       'voice_mode', // 0: Poly, 1: Mono, 2: Legato

    // Amp Envelope
    AMP_ENV_ATTACK: 'amp_env_attack',
    AMP_ENV_ATTACK_SLOPE: 'amp_env_attack_slope',
    AMP_ENV_DECAY: 'amp_env_decay',
    AMP_ENV_DECAY_SLOPE: 'amp_env_decay_slope',
    AMP_ENV_SUSTAIN: 'amp_env_sustain',
    AMP_ENV_RELEASE: 'amp_env_release',
    AMP_ENV_RELEASE_SLOPE: 'amp_env_release_slope',

    // Filter Envelope
    FILTER_ENV_ATTACK: 'filter_env_attack',
    FILTER_ENV_ATTACK_SLOPE: 'filter_env_attack_slope',
    FILTER_ENV_DECAY: 'filter_env_decay',
    FILTER_ENV_DECAY_SLOPE: 'filter_env_decay_slope',
    FILTER_ENV_SUSTAIN: 'filter_env_sustain',
    FILTER_ENV_RELEASE: 'filter_env_release',
    FILTER_ENV_RELEASE_SLOPE: 'filter_env_release_slope',
    FILTER_ENV_OSC1: 'filter_env_osc1',
    FILTER_ENV_OSC2: 'filter_env_osc2',
    FILTER_ENV_OSC3: 'filter_env_osc3',

    // Pitch Envelope
    PITCH_ENV_ATTACK: 'pitch_env_attack',
    PITCH_ENV_ATTACK_SLOPE: 'pitch_env_attack_slope',
    PITCH_ENV_DECAY: 'pitch_env_decay',
    PITCH_ENV_DECAY_SLOPE: 'pitch_env_decay_slope',
    PITCH_ENV_SUSTAIN: 'pitch_env_sustain',
    PITCH_ENV_RELEASE: 'pitch_env_release',
    PITCH_ENV_RELEASE_SLOPE: 'pitch_env_release_slope',
    PITCH_ENV_OSC1: 'pitch_env_osc1',
    PITCH_ENV_OSC2: 'pitch_env_osc2',
    PITCH_ENV_OSC3: 'pitch_env_osc3',
    PITCH_ENV_AMOUNT: 'pitch_env_amount',

    // Mod Envelope
    MOD_ENV_ATTACK: 'mod_env_attack',
    MOD_ENV_ATTACK_SLOPE: 'mod_env_attack_slope',
    MOD_ENV_DECAY: 'mod_env_decay',
    MOD_ENV_DECAY_SLOPE: 'mod_env_decay_slope',
    MOD_ENV_SUSTAIN: 'mod_env_sustain',
    MOD_ENV_RELEASE: 'mod_env_release',
    MOD_ENV_RELEASE_SLOPE: 'mod_env_release_slope',
    MOD_ENV_SHAPE1: 'mod_env_shape1',
    MOD_ENV_SHAPE2: 'mod_env_shape2',
    MOD_ENV_SHAPE3: 'mod_env_shape3',
    MOD_ENV_FM: 'mod_env_fm',

    // Preset selector
    PRESET_INDEX: 'preset_index'
});