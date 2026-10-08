#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SPCTRLDAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SPCTRLDAudioProcessorEditor(SPCTRLDAudioProcessor&);
    ~SPCTRLDAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    SPCTRLDAudioProcessor& processor;

    juce::Image logo;
    juce::Label title;
    juce::Label distSection, compSection, clipSection;
    juce::Slider drive, asym, tone, mix;
    juce::Slider compThresh, compRatio, compAttack, compRelease, compMakeup;
    juce::Slider clip, clipMix, output;
    juce::Label driveLabel, asymLabel, toneLabel, mixLabel;
    juce::Label compThreshLabel, compRatioLabel, compAttackLabel, compReleaseLabel, compMakeupLabel;
    juce::Label clipLabel, clipMixLabel, outputLabel;

    std::vector<std::unique_ptr<SliderAttachment>> attachments;
    void setupSlider(juce::Slider&, juce::Label&, const juce::String&);
    void setupSectionLabel(juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPCTRLDAudioProcessorEditor)
};
