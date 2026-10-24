#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include <vector>

// We need the full definition of JolleyBrowserComponent because it is 
// stored by value (not by pointer/reference) in the member list below.
#include "JolleyBrowserComponent.h"

// Forward declaration is sufficient for the reference member.
class JolleyAudioProcessor;

class JolleyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    using Options    = juce::WebBrowserComponent::Options;
    using Relay      = juce::WebSliderRelay;                
    using Attachment = juce::WebSliderParameterAttachment;  

    explicit JolleyAudioProcessorEditor (JolleyAudioProcessor&);
    ~JolleyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // These must be static so they can be called from the initializer list 
    // before the `this` pointer is fully constructed.
    static std::vector<std::unique_ptr<Relay>> createRelays (JolleyAudioProcessor&);
    static std::vector<std::unique_ptr<Attachment>>
        createAttachments (JolleyAudioProcessor&, const std::vector<std::unique_ptr<Relay>>&);

    JolleyAudioProcessor& audioProcessor;

    // ===== ORDER IS LOAD-BEARING =====
    // C++ initialises members in *declaration* order, not initialiser-list order.
    // 1. relays must come first so they exist when webView's Options are built.
    // 2. webView is constructed using the options built from the relays.
    // 3. attachments must come last because they reference both the processor and the relays.
    std::vector<std::unique_ptr<Relay>>      relays;
    JolleyBrowserComponent                   webView;
    std::vector<std::unique_ptr<Attachment>> attachments;
    // =====================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JolleyAudioProcessorEditor)
};