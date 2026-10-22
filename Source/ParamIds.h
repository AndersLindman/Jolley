#pragma once
#include <string_view>

namespace ParamIDs
{
    // Master volume
    inline constexpr std::string_view MASTER_VOLUME = "master_volume";

    // Tube drive
    inline constexpr std::string_view TUBE_DRIVE = "tube_drive";

    // Oscillator 1
    inline constexpr std::string_view OSC1_SHAPE = "osc1_shape";
    inline constexpr std::string_view OSC1_OCTAVE = "osc1_octave";
    inline constexpr std::string_view OSC1_SEMITONE = "osc1_semitone";
    inline constexpr std::string_view OSC1_FINE = "osc1_fine";
    inline constexpr std::string_view OSC1_ROLLOFF = "osc1_rolloff";
    inline constexpr std::string_view OSC1_PAN = "osc1_pan";
    inline constexpr std::string_view OSC1_LEVEL = "osc1_level";

    // Oscillator 2
    inline constexpr std::string_view OSC2_SHAPE = "osc2_shape";
    inline constexpr std::string_view OSC2_OCTAVE = "osc2_octave";
    inline constexpr std::string_view OSC2_SEMITONE = "osc2_semitone";
    inline constexpr std::string_view OSC2_FINE = "osc2_fine";
    inline constexpr std::string_view OSC2_ROLLOFF = "osc2_rolloff";
    inline constexpr std::string_view OSC2_PAN = "osc2_pan";
    inline constexpr std::string_view OSC2_LEVEL = "osc2_level";

    // Oscillator 3
    inline constexpr std::string_view OSC3_SHAPE = "osc3_shape";
    inline constexpr std::string_view OSC3_OCTAVE = "osc3_octave";
    inline constexpr std::string_view OSC3_SEMITONE = "osc3_semitone";
    inline constexpr std::string_view OSC3_FINE = "osc3_fine";
    inline constexpr std::string_view OSC3_ROLLOFF = "osc3_rolloff";
    inline constexpr std::string_view OSC3_PAN = "osc3_pan";
    inline constexpr std::string_view OSC3_LEVEL = "osc3_level";

    // Sync Toggles
    inline constexpr std::string_view OSC2_SYNC_TO_OSC1 = "osc2_sync_to_osc1";
    inline constexpr std::string_view OSC3_SYNC_TO_OSC1 = "osc3_sync_to_osc1";

    // FM Matrix IDs
    inline constexpr std::string_view FM_ACTIVE = "fm_active";
    inline constexpr std::string_view FM_OSC3_RATIO = "fm_osc3_ratio";
    inline constexpr std::string_view FM_DEPTH_3TO2 = "fm_depth_3to2";
    inline constexpr std::string_view FM_OSC3_FEEDBACK = "fm_osc3_feedback";
    inline constexpr std::string_view FM_OSC2_RATIO = "fm_osc2_ratio";
    inline constexpr std::string_view FM_DEPTH_2TO1 = "fm_depth_2to1";
    inline constexpr std::string_view FM_MASTER_AMOUNT = "fm_master_amount";

    // LFO 1
    inline constexpr std::string_view LFO1_RATE            = "lfo1_rate";
    inline constexpr std::string_view LFO1_DEPTH           = "lfo1_depth";
    inline constexpr std::string_view LFO1_SHAPE           = "lfo1_shape";
    inline constexpr std::string_view LFO1_DEST_OSC1_PITCH = "lfo1_dest_osc1_pitch";
    inline constexpr std::string_view LFO1_DEST_OSC1_SHAPE = "lfo1_dest_osc1_shape";
    inline constexpr std::string_view LFO1_DEST_CUTOFF     = "lfo1_dest_cutoff";

    // LFO 2
    inline constexpr std::string_view LFO2_RATE            = "lfo2_rate";
    inline constexpr std::string_view LFO2_DEPTH           = "lfo2_depth";
    inline constexpr std::string_view LFO2_SHAPE           = "lfo2_shape";
    inline constexpr std::string_view LFO2_DEST_OSC2_PITCH = "lfo2_dest_osc2_pitch";
    inline constexpr std::string_view LFO2_DEST_OSC2_SHAPE = "lfo2_dest_osc2_shape";
    inline constexpr std::string_view LFO2_DEST_FM_AMOUNT  = "lfo2_dest_fm_amount";

    // Filter IDs
    inline constexpr std::string_view FILTER_TYPE        = "filter_type";
    inline constexpr std::string_view FILTER_CUTOFF      = "filter_cutoff";
    inline constexpr std::string_view FILTER_RESONANCE   = "filter_resonance";
    inline constexpr std::string_view FILTER_DRIVE       = "filter_drive";
    inline constexpr std::string_view FILTER_FEEDBACK    = "filter_feedback";
    inline constexpr std::string_view FILTER_MORPH       = "filter_morph";
    inline constexpr std::string_view FILTER_ENV_AMOUNT  = "filter_env_amount";

    // Unison & Voice Character
    inline constexpr std::string_view UNISON_VOICES = "unison_voices";
    inline constexpr std::string_view UNISON_DETUNE = "unison_detune";
    inline constexpr std::string_view UNISON_SPREAD = "unison_spread";

    // Performance & Expressivity
    inline constexpr std::string_view VOICE_GLIDE       = "voice_glide";
    inline constexpr std::string_view VOICE_BEND        = "voice_bend";
    inline constexpr std::string_view VOICE_VELOCITY    = "voice_velocity";
    inline constexpr std::string_view VOICE_MOD_CUTOFF  = "voice_mod_cutoff";
    inline constexpr std::string_view VOICE_DRIFT       = "voice_drift";

    // Voice Mode State
    inline constexpr std::string_view VOICE_MODE = "voice_mode";

    // Amp Envelope
    inline constexpr std::string_view AMP_ENV_ATTACK       = "amp_env_attack";
    inline constexpr std::string_view AMP_ENV_ATTACK_SLOPE = "amp_env_attack_slope";
    inline constexpr std::string_view AMP_ENV_DECAY        = "amp_env_decay";
    inline constexpr std::string_view AMP_ENV_DECAY_SLOPE  = "amp_env_decay_slope";
    inline constexpr std::string_view AMP_ENV_SUSTAIN      = "amp_env_sustain";
    inline constexpr std::string_view AMP_ENV_RELEASE      = "amp_env_release";
    inline constexpr std::string_view AMP_ENV_RELEASE_SLOPE = "amp_env_release_slope";

    // Filter Envelope
    inline constexpr std::string_view FILTER_ENV_ATTACK       = "filter_env_attack";
    inline constexpr std::string_view FILTER_ENV_ATTACK_SLOPE = "filter_env_attack_slope";
    inline constexpr std::string_view FILTER_ENV_DECAY        = "filter_env_decay";
    inline constexpr std::string_view FILTER_ENV_DECAY_SLOPE  = "filter_env_decay_slope";
    inline constexpr std::string_view FILTER_ENV_SUSTAIN      = "filter_env_sustain";
    inline constexpr std::string_view FILTER_ENV_RELEASE      = "filter_env_release";
    inline constexpr std::string_view FILTER_ENV_RELEASE_SLOPE = "filter_env_release_slope";
    inline constexpr std::string_view FILTER_ENV_OSC1         = "filter_env_osc1";
    inline constexpr std::string_view FILTER_ENV_OSC2         = "filter_env_osc2";
    inline constexpr std::string_view FILTER_ENV_OSC3         = "filter_env_osc3";

    // Pitch Envelope
    inline constexpr std::string_view PITCH_ENV_ATTACK       = "pitch_env_attack";
    inline constexpr std::string_view PITCH_ENV_ATTACK_SLOPE = "pitch_env_attack_slope";
    inline constexpr std::string_view PITCH_ENV_DECAY        = "pitch_env_decay";
    inline constexpr std::string_view PITCH_ENV_DECAY_SLOPE  = "pitch_env_decay_slope";
    inline constexpr std::string_view PITCH_ENV_SUSTAIN      = "pitch_env_sustain";
    inline constexpr std::string_view PITCH_ENV_RELEASE      = "pitch_env_release";
    inline constexpr std::string_view PITCH_ENV_RELEASE_SLOPE = "pitch_env_release_slope";
    inline constexpr std::string_view PITCH_ENV_OSC1         = "pitch_env_osc1";
    inline constexpr std::string_view PITCH_ENV_OSC2         = "pitch_env_osc2";
    inline constexpr std::string_view PITCH_ENV_OSC3         = "pitch_env_osc3";
    inline constexpr std::string_view PITCH_ENV_AMOUNT       = "pitch_env_amount";

    // Mod Envelope
    inline constexpr std::string_view MOD_ENV_ATTACK       = "mod_env_attack";
    inline constexpr std::string_view MOD_ENV_ATTACK_SLOPE = "mod_env_attack_slope";
    inline constexpr std::string_view MOD_ENV_DECAY        = "mod_env_decay";
    inline constexpr std::string_view MOD_ENV_DECAY_SLOPE  = "mod_env_decay_slope";
    inline constexpr std::string_view MOD_ENV_SUSTAIN      = "mod_env_sustain";
    inline constexpr std::string_view MOD_ENV_RELEASE      = "mod_env_release";
    inline constexpr std::string_view MOD_ENV_RELEASE_SLOPE = "mod_env_release_slope";
    inline constexpr std::string_view MOD_ENV_SHAPE1     = "mod_env_shape1";
    inline constexpr std::string_view MOD_ENV_SHAPE2     = "mod_env_shape2";
    inline constexpr std::string_view MOD_ENV_SHAPE3     = "mod_env_shape3";
    inline constexpr std::string_view MOD_ENV_FM         = "mod_env_fm";

    // Preset Selector
    inline constexpr std::string_view PRESET_INDEX = "preset_index";
}