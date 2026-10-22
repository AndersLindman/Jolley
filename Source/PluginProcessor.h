#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/synth/SynthEngine.h"

class JolleyAudioProcessor  : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener // <--- Clean inheritance
{
public:
    static constexpr int DEFAULT_WIDTH  = 1000;
    static constexpr int DEFAULT_HEIGHT = 660;
    static constexpr int MIN_WIDTH      = 400;
    static constexpr int MIN_HEIGHT     = 264;
    static constexpr int MAX_WIDTH      = 3840;
    static constexpr int MAX_HEIGHT     = 2534;
    
    JolleyAudioProcessor();
    ~JolleyAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState parameters;

private: 
    SynthEngine synthEngine;
    void renderSynthEngine (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages);
    void JolleyAudioProcessor::renderSynthSubBlock(juce::AudioBuffer<double>& buffer, int startSample, int numSamples);
    juce::AudioBuffer<double> doubleScratchBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JolleyAudioProcessor)
};