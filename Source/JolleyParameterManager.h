#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include <vector>
#include <memory>

class JolleyBrowserComponent : public juce::WebBrowserComponent
{
public:
    explicit JolleyBrowserComponent (JolleyAudioProcessor& processor, const Options& options)
        : juce::WebBrowserComponent (buildOptionsWithRelays (processor, options))
    {
        // Attachments are created after base constructor finishes so they bind correctly
        auto params = processor.parameters.processor.getParameters();
        for (size_t i = 0; i < relays.size() && i < params.size(); ++i)
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*> (params[i]))
            {
                auto attachment = std::make_unique<juce::WebSliderParameterAttachment> (
                    *rangedParam, *relays[i]
                );
                attachments.push_back (std::move (attachment));
            }
        }
    }

    bool pageAboutToLoad (const juce::String& newURL) override
    {
        // Allow local Vite dev server or resource provider
        return newURL.startsWith ("http://localhost") || newURL.startsWith ("http://127.0.0.1") || newURL == getResourceProviderRoot();
    }

private:
    std::vector<std::unique_ptr<juce::WebSliderRelay>> relays;
    std::vector<std::unique_ptr<juce::WebSliderParameterAttachment>> attachments;

    // Static helper to build options and populate relays *before* base constructor runs
    Options buildOptionsWithRelays (JolleyAudioProcessor& processor, Options options)
    {
        for (auto* param : processor.parameters.processor.getParameters())
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*> (param))
            {
                auto paramID = rangedParam->getParameterID();
                auto relay = std::make_unique<juce::WebSliderRelay> (*this, paramID);
                options.withOptionsFrom (*relay);
                relays.push_back (std::move (relay));
            }
        }
        return options;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JolleyBrowserComponent)
};