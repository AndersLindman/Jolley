#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include "ParamIds.h"

JolleyAudioProcessor::JolleyAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    if (! parameters.state.hasProperty ("uiWidth"))
        parameters.state.setProperty ("uiWidth", DEFAULT_WIDTH, nullptr);
    if (! parameters.state.hasProperty ("uiHeight"))
        parameters.state.setProperty ("uiHeight",DEFAULT_HEIGHT, nullptr);

    // Automatically add this class as a listener to ALL parameters in the layout.
    for (const auto& tree : parameters.state)
    {
        parameters.addParameterListener (tree.getProperty (juce::Identifier ("id")).toString(), this);
    }

    // --- AUTOMATIC INITIALIZATION SYNC (Normalized Values) ---
    for (const auto& tree : parameters.state)
    {
        auto parameterID = tree.getProperty (juce::Identifier ("id")).toString();
        
        if (auto* rawParam = parameters.getParameter (parameterID))
        {
            parameterChanged (parameterID, rawParam->getValue());
        }
    }
}

JolleyAudioProcessor::~JolleyAudioProcessor()
{
    // Automatically remove listeners to prevent crashes on destruction
    for (const auto& tree : parameters.state)
    {
        parameters.removeParameterListener (tree.getProperty (juce::Identifier ("id")).toString(), this);
    }
}

const juce::String JolleyAudioProcessor::getName() const { return JucePlugin_Name; }
bool JolleyAudioProcessor::acceptsMidi() const { return true; }
bool JolleyAudioProcessor::producesMidi() const { return false; }
bool JolleyAudioProcessor::isMidiEffect() const { return false; }
double JolleyAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int JolleyAudioProcessor::getNumPrograms() { return 1; }
int JolleyAudioProcessor::getCurrentProgram() { return 0; }
void JolleyAudioProcessor::setCurrentProgram (int) {}
const juce::String JolleyAudioProcessor::getProgramName (int) { return {}; }
void JolleyAudioProcessor::changeProgramName (int, const juce::String&) {}

void JolleyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create an XML representation of all parameters stored in the APVTS
    auto state = parameters.copyState();
    
    // Convert the ValueTree to an XML document
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    
    // Copy the XML data into the DAW's binary memory block
    copyXmlToBinary (*xml, destData);
}

void JolleyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Parse the binary data coming from the DAW back into an XML document
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    // If parsing succeeds and it's a valid ValueTree XML root, restore it
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

// Pre-allocate memory here. This is allowed to block because it runs on the main thread.
void JolleyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    doubleScratchBuffer.setSize (getMainBusNumOutputChannels(), samplesPerBlock, false, false, true);
    synthEngine.prepare(sampleRate, samplesPerBlock, getMainBusNumOutputChannels());
}

bool JolleyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
   && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
      return false;
  return true;
}

void JolleyAudioProcessor::releaseResources()
{
    // When the DAW stops, bypasses, or unloads the plugin, 
    // instantly kill all voices to prevent hanging notes.
    // 'true' = immediate cut (All Sound Off behavior)
    synthEngine.allNotesOff(true);
}

void JolleyAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    renderSynthEngine (buffer, midiMessages);
}

void JolleyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // We just clear the pre-allocated buffer.
    doubleScratchBuffer.clear();

    renderSynthEngine (doubleScratchBuffer, midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* floatOut = buffer.getWritePointer (channel);
        const auto* doubleIn = doubleScratchBuffer.getReadPointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            floatOut[sample] = static_cast<float> (doubleIn[sample]);
        }
    }
}

void JolleyAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused (newValue);
    if (auto* param = parameters.getParameter (parameterID))
    {
        // BOUNDARY CAST: float -> double immediately before hitting DSP
        synthEngine.setParam (parameterID, static_cast<double>(param->getValue()));
    }
}

void JolleyAudioProcessor::renderSynthEngine(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    const int numSamples = buffer.getNumSamples();
    int currentSample = 0;

    for (const auto metadata : midiMessages)
    {
        int targetSample = std::clamp(metadata.samplePosition, 0, numSamples);

        if (targetSample > currentSample)
        {
            renderSynthSubBlock(buffer, currentSample, targetSample - currentSample);
            currentSample = targetSample;
        }

        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            // BOUNDARY CAST: float -> double immediately
            synthEngine.noteOn(message.getNoteNumber(), static_cast<double>(message.getFloatVelocity()));
        }
        else if (message.isNoteOff())
        {
            synthEngine.noteOff(message.getNoteNumber());
        }
        else if (message.isController() && message.getControllerNumber() == 1) // Mod Wheel
        {
            // BOUNDARY CAST: int -> double immediately
            double modWheelNormalized = static_cast<double>(message.getControllerValue()) / 127.0;
            synthEngine.setModWheel(modWheelNormalized);
        }
        else if (message.isController() && (message.getControllerNumber() == 123 || message.getControllerNumber() == 124))
        {
            synthEngine.allNotesOff(message.getControllerNumber() == 124);
        }
        else if (message.isPitchWheel())
        {
            // BOUNDARY CAST: int -> double immediately
            int rawPitchWheel = message.getPitchWheelValue();
            double pitchBendNormalized = static_cast<double>(rawPitchWheel - 8192) / 8192.0;
            synthEngine.setPitchBend(pitchBendNormalized);
        }
    }

    if (currentSample < numSamples)
    {
        renderSynthSubBlock(buffer, currentSample, numSamples - currentSample);
    }
}

// Helper method to render a specific slice of the audio buffer
void JolleyAudioProcessor::renderSynthSubBlock(juce::AudioBuffer<double>& buffer, int startSample, int numSamples)
{
    if (numSamples <= 0) 
        return;

    auto* leftChannel = buffer.getWritePointer(0) + startSample;
    auto* rightChannel = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) + startSample : nullptr;

    synthEngine.processSubBlock(leftChannel, rightChannel, numSamples);
}

bool JolleyAudioProcessor::hasEditor() const { return true; } 

juce::AudioProcessorEditor* JolleyAudioProcessor::createEditor() 
{ 
    return new JolleyAudioProcessorEditor (*this); 
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JolleyAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout JolleyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Helper lambda using double precision arguments, mapped down to float for JUCE
    auto addParam = [&](std::string_view id, std::string name, double defaultValue, double min, double max, double interval = 0.0)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ id.data(), 1 },
            name,
            juce::NormalisableRange<float>(static_cast<float>(min), 
                                           static_cast<float>(max), 
                                           static_cast<float>(interval)),
            static_cast<float>(defaultValue)
        ));
    };

    // --- Master & Tube Drive ---
    addParam(ParamIDs::MASTER_VOLUME, "Master Volume", 0.7, 0.0, 1.0);
    addParam(ParamIDs::TUBE_DRIVE,    "Tube Drive",    0.0, 0.0, 1.0);

    // --- Oscillators 1, 2, and 3 (Automated Loop) ---
    for (int oscNr = 1; oscNr <= 3; ++oscNr)
    {
        std::string prefix = "Osc " + std::to_string(oscNr) + " ";
        std::string idPrefix = "osc" + std::to_string(oscNr) + "_";

        addParam(idPrefix + "shape",    prefix + "Shape",    1.0,   0.0,   1.0);
        addParam(idPrefix + "octave",   prefix + "Octave",   0.0,  -2.0,   2.0,  1.0);
        addParam(idPrefix + "semitone", prefix + "Semitone", 0.0, -12.0,  12.0,  1.0);
        addParam(idPrefix + "fine",     prefix + "Fine",     0.0, -50.0,  50.0);
        addParam(idPrefix + "rolloff",  prefix + "Roll-off", 0.98,  0.0,   1.0);
        addParam(idPrefix + "pan",      prefix + "Pan",      0.0,  -1.0,   1.0);
        
        double defaultLevel = (oscNr == 1) ? 0.7 : 0.0;
        addParam(idPrefix + "level",    prefix + "Level",    defaultLevel, 0.0, 1.0);
    }

    // --- Sync Toggles ---
    addParam(ParamIDs::OSC2_SYNC_TO_OSC1, "Sync Osc 2 to Osc 1", 0.0, 0.0, 1.0, 1.0);
    addParam(ParamIDs::OSC3_SYNC_TO_OSC1, "Sync Osc 3 to Osc 1", 0.0, 0.0, 1.0, 1.0);

    // --- FM Matrix ---
    addParam(ParamIDs::FM_ACTIVE,          "Active",             0.0, 0.0,  1.0);
    addParam(ParamIDs::FM_OSC3_RATIO,      "Osc 3 FM Ratio",    1.0, 0.0, 12.0, 1.0);
    addParam(ParamIDs::FM_DEPTH_3TO2,      "FM Depth 3 -> 2",   0.0, 0.0,  1.0);
    addParam(ParamIDs::FM_OSC3_FEEDBACK,   "Osc 3 Feedback",    0.0, 0.0,  1.0);
    addParam(ParamIDs::FM_OSC2_RATIO,      "Osc 2 FM Ratio",    1.0, 0.0, 12.0, 1.0);
    addParam(ParamIDs::FM_DEPTH_2TO1,      "FM Depth 2 -> 1",   0.0, 0.0,  1.0);
    addParam(ParamIDs::FM_MASTER_AMOUNT,   "Master FM Amount",  0.0, 0.0,  1.0);

    // --- LFO 1 ---
    addParam(ParamIDs::LFO1_RATE,            "LFO 1 Rate",            1.0, 0.1, 20.0);
    addParam(ParamIDs::LFO1_DEPTH,           "LFO 1 Depth",           0.0, 0.0,  1.0);
    addParam(ParamIDs::LFO1_SHAPE,           "LFO 1 Shape",           0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO1_DEST_OSC1_PITCH, "LFO 1 -> Osc 1 Pitch", 0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO1_DEST_OSC1_SHAPE, "LFO 1 -> Osc 1 Shape", 0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO1_DEST_CUTOFF,     "LFO 1 -> Filter Cutoff", 0.0, 0.0, 1.0, 1.0);

    // --- LFO 2 ---
    addParam(ParamIDs::LFO2_RATE,            "LFO 2 Rate",            1.0, 0.1, 20.0);
    addParam(ParamIDs::LFO2_DEPTH,           "LFO 2 Depth",           0.0, 0.0,  1.0);
    addParam(ParamIDs::LFO2_SHAPE,           "LFO 2 Shape",           0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO2_DEST_OSC2_PITCH, "LFO 2 -> Osc 2 Pitch", 0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO2_DEST_OSC2_SHAPE, "LFO 2 -> Osc 2 Shape", 0.0, 0.0,  1.0, 1.0);
    addParam(ParamIDs::LFO2_DEST_FM_AMOUNT,  "LFO 2 -> FM Master Amount", 0.0, 0.0, 1.0, 1.0);

    // --- Filter Parameters ---
    addParam(ParamIDs::FILTER_TYPE,      "Filter Type",      0.0,  0.0,    2.0,  1.0);
    addParam(ParamIDs::FILTER_CUTOFF,    "Filter Cutoff",    20000.0, 20.0, 20000.0);
    addParam(ParamIDs::FILTER_RESONANCE, "Filter Resonance", 0.0,  0.0,    1.0);
    addParam(ParamIDs::FILTER_DRIVE,     "Filter Drive",     0.0,  0.0,    1.0);
    addParam(ParamIDs::FILTER_FEEDBACK,  "Filter Feedback",  0.0,  0.0,    1.0);
    addParam(ParamIDs::FILTER_MORPH,     "Filter Morph",     0.0,  0.0,    1.0);
    addParam(ParamIDs::FILTER_ENV_AMOUNT,"Filter Env Depth", 0.0, -1.0,    1.0);

    // --- Unison & Voice Character ---
    addParam(ParamIDs::VOICE_MODE,       "Voice Mode",           0.0,  0.0,    2.0,  1.0);
    addParam(ParamIDs::UNISON_VOICES,    "Unison Voices",        1.0,  1.0,    8.0,  1.0);
    addParam(ParamIDs::UNISON_DETUNE,    "Unison Detune",        15.0,  0.0,  100.0);
    addParam(ParamIDs::UNISON_SPREAD,    "Unison Stereo Spread", 0.8, 0.0,    1.0);
    addParam(ParamIDs::VOICE_DRIFT,     "Analog Pitch Drift",   0.0, 0.0,    1.0);
    addParam(ParamIDs::VOICE_GLIDE,      "Portamento Glide",     0.0, 0.0, 500.0);
    addParam(ParamIDs::VOICE_BEND,       "Pitch Bend Range",     2.0, 0.0,   24.0,  1.0);
    addParam(ParamIDs::VOICE_VELOCITY,   "Velocity Depth",       0.5, 0.0,    1.0);
    addParam(ParamIDs::VOICE_MOD_CUTOFF, "Mod Wheel Cutoff",     0.0, -1.0,    1.0);

    // --- Amp Envelope ---
    addParam(ParamIDs::AMP_ENV_ATTACK,        "Amp Env Attack",        0.005,  0.001, 10.0);
    addParam(ParamIDs::AMP_ENV_ATTACK_SLOPE,  "Amp Env Attack Slope",   3.0,  -20.0,   20.0);
    addParam(ParamIDs::AMP_ENV_DECAY,         "Amp Env Decay",          0.2,    0.001, 10.0);
    addParam(ParamIDs::AMP_ENV_DECAY_SLOPE,   "Amp Env Decay Slope",    3.0,  -20.0,   20.0);
    addParam(ParamIDs::AMP_ENV_SUSTAIN,       "Amp Env Sustain",        0.7,    0.0,     1.0);
    addParam(ParamIDs::AMP_ENV_RELEASE,       "Amp Env Release",        0.25,    0.001, 10.0);
    addParam(ParamIDs::AMP_ENV_RELEASE_SLOPE, "Amp Env Release Slope", 3.0,  -20.0,   20.0);

    // --- Filter Envelope ---
    addParam(ParamIDs::FILTER_ENV_ATTACK,         "Filter Env Attack",         0.01,  0.001, 10.0);
    addParam(ParamIDs::FILTER_ENV_ATTACK_SLOPE,  "Filter Env Attack Slope",  3.0, -20.0,   20.0);
    addParam(ParamIDs::FILTER_ENV_DECAY,         "Filter Env Decay",          0.3,   0.001, 10.0);
    addParam(ParamIDs::FILTER_ENV_DECAY_SLOPE,   "Filter Env Decay Slope",   3.0, -20.0,   20.0);
    addParam(ParamIDs::FILTER_ENV_SUSTAIN,       "Filter Env Sustain",       0.5,   0.0,    1.0);
    addParam(ParamIDs::FILTER_ENV_RELEASE,       "Filter Env Release",       0.25,   0.001, 10.0);
    addParam(ParamIDs::FILTER_ENV_RELEASE_SLOPE, "Filter Env Release Slope", 3.0, -20.0,   20.0);
    addParam(ParamIDs::FILTER_ENV_OSC1,          "Filter Env to Osc 1",      1.0,   0.0,    1.0);
    addParam(ParamIDs::FILTER_ENV_OSC2,          "Filter Env to Osc 2",      1.0,   0.0,    1.0);
    addParam(ParamIDs::FILTER_ENV_OSC3,          "Filter Env to Osc 3",      1.0,   0.0,    1.0);

    // --- Pitch Envelope ---
    addParam(ParamIDs::PITCH_ENV_ATTACK,        "Pitch Env Attack",         0.01,  0.001, 10.0);
    addParam(ParamIDs::PITCH_ENV_ATTACK_SLOPE,  "Pitch Env Attack Slope",  3.0, -20.0,   20.0);
    addParam(ParamIDs::PITCH_ENV_DECAY,         "Pitch Env Decay",          0.3,   0.001, 10.0);
    addParam(ParamIDs::PITCH_ENV_DECAY_SLOPE,   "Pitch Env Decay Slope",   3.0, -20.0,   20.0);
    addParam(ParamIDs::PITCH_ENV_SUSTAIN,       "Pitch Env Sustain",       0.5,   0.0,    1.0);
    addParam(ParamIDs::PITCH_ENV_RELEASE,       "Pitch Env Release",       0.25,   0.001, 10.0);
    addParam(ParamIDs::PITCH_ENV_RELEASE_SLOPE, "Pitch Env Release Slope", 3.0, -20.0,   20.0);
    addParam(ParamIDs::PITCH_ENV_OSC1,          "Pitch to Osc 1",           0.0,   0.0,    1.0);
    addParam(ParamIDs::PITCH_ENV_OSC2,          "Pitch Env to Osc 2",       0.0,   0.0,    1.0);
    addParam(ParamIDs::PITCH_ENV_OSC3,          "Pitch Env to Osc 3",       0.0,   0.0,    1.0);
    addParam(ParamIDs::PITCH_ENV_AMOUNT,        "Pitch Env Amount",         0.5,   0.0,    1.0);

    // --- Mod Envelope ---
    addParam(ParamIDs::MOD_ENV_ATTACK,          "Mod Env Attack",         0.01,  0.001, 10.0);
    addParam(ParamIDs::MOD_ENV_ATTACK_SLOPE,    "Mod Env Attack Slope",  3.0, -20.0,   20.0);
    addParam(ParamIDs::MOD_ENV_DECAY,           "Mod Env Decay",          0.3,   0.001, 10.0);
    addParam(ParamIDs::MOD_ENV_DECAY_SLOPE,     "Mod Env Decay Slope",   3.0, -20.0,   20.0);
    addParam(ParamIDs::MOD_ENV_SUSTAIN,         "Mod Env Sustain",       0.5,   0.0,    1.0);
    addParam(ParamIDs::MOD_ENV_RELEASE,         "Mod Env Release",       0.25,   0.001, 10.0);
    addParam(ParamIDs::MOD_ENV_RELEASE_SLOPE,   "Mod Env Release Slope", 3.0, -20.0,   20.0);
    addParam(ParamIDs::MOD_ENV_SHAPE1,          "Mod Env to Osc 1 Shape", 0.0, 0.0, 1.0);
    addParam(ParamIDs::MOD_ENV_SHAPE2,          "Mod Env to Osc 2 Shape", 0.0, 0.0, 1.0);
    addParam(ParamIDs::MOD_ENV_SHAPE3,          "Mod Env to Osc 3 Shape", 0.0, 0.0, 1.0);
    addParam(ParamIDs::MOD_ENV_FM,              "Mod Env to FM Amount",   0.0, 0.0, 1.0);

    
    // --- Preset Selector ---
    addParam(ParamIDs::PRESET_INDEX,         "Preset Index",         0.0,  0.0, 1.0);

    return layout;
}