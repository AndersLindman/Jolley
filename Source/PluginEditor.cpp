#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <memory>
#include <vector>

using namespace juce;

//==============================================================================
namespace
{
    // Builds the browser Options and registers every relay with them.
    //
    // This is called from the constructor initialiser list while `webView` has
    // not been constructed yet. That is safe: WebSliderRelay's constructor only
    // binds the reference, and buildOptions() never touches the browser.
    JolleyAudioProcessorEditor::Options createBrowserOptions (
        const std::vector<std::unique_ptr<JolleyAudioProcessorEditor::Relay>>& relays)
    {
        auto options = JolleyBrowserComponent::createDefaultOptions();

        for (const auto& relay : relays)
            options = options.withOptionsFrom (*relay);

        return options;
    }
}

JolleyAudioProcessorEditor::JolleyAudioProcessorEditor (JolleyAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      relays      (createRelays (p, webView)),
      webView     (createBrowserOptions (relays)),
      attachments (createAttachments (p, relays))
{
    addAndMakeVisible (webView);

   #if JUCE_DEBUG
    webView.goToURL ("http://127.0.0.1:5173/");
   #else
    webView.goToURL (WebBrowserComponent::getResourceProviderRoot());
   #endif

    setResizable (true, true);
    setResizeLimits (JolleyAudioProcessor::MIN_WIDTH,
                     JolleyAudioProcessor::MIN_HEIGHT,
                     JolleyAudioProcessor::MAX_WIDTH,
                     JolleyAudioProcessor::MAX_HEIGHT);

    // audioProcessor.parameters.state is already a ValueTree
    const auto& uiTree = audioProcessor.parameters.state;

    const int savedWidth  = uiTree.getProperty ("uiWidth",  JolleyAudioProcessor::DEFAULT_WIDTH);
    const int savedHeight = uiTree.getProperty ("uiHeight", JolleyAudioProcessor::DEFAULT_HEIGHT);

    setSize (jlimit (JolleyAudioProcessor::MIN_WIDTH,  JolleyAudioProcessor::MAX_WIDTH,  savedWidth),
             jlimit (JolleyAudioProcessor::MIN_HEIGHT, JolleyAudioProcessor::MAX_HEIGHT, savedHeight));
}

JolleyAudioProcessorEditor::~JolleyAudioProcessorEditor()
{
    // audioProcessor.parameters.state is already a ValueTree
    auto& uiTree = audioProcessor.parameters.state;

    uiTree.setProperty ("uiWidth",  getWidth(),  nullptr);
    uiTree.setProperty ("uiHeight", getHeight(), nullptr);
}

std::vector<std::unique_ptr<JolleyAudioProcessorEditor::Relay>>
JolleyAudioProcessorEditor::createRelays (JolleyAudioProcessor& processor,
                                          WebBrowserComponent& browser)
{
    std::vector<std::unique_ptr<Relay>> result;

   for (auto* param : processor.parameters.processor.getParameters())
        if (auto* rangedParam = dynamic_cast<RangedAudioParameter*> (param))
          result.push_back (std::make_unique<Relay> (browser, rangedParam->getParameterID()));

    return result;
}

std::vector<std::unique_ptr<JolleyAudioProcessorEditor::Attachment>>
JolleyAudioProcessorEditor::createAttachments (JolleyAudioProcessor& processor,
                                               const std::vector<std::unique_ptr<Relay>>& relays)
{
    std::vector<std::unique_ptr<Attachment>> result;

    // Both loops use the identical filter, so indices line up 1:1.
    size_t i = 0;

    for (auto* param : processor.parameters.processor.getParameters())
    {
        if (auto* rangedParam = dynamic_cast<RangedAudioParameter*> (param))
        {
            if (i < relays.size())
                result.push_back (std::make_unique<Attachment> (*rangedParam, *relays[i]));

            ++i;
        }
    }

    return result;
}

void JolleyAudioProcessorEditor::paint (Graphics& g)
{
    // The editor is opaque - fill it so nothing flashes through before the
    // web view has loaded (or if the bundle is missing).
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void JolleyAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}