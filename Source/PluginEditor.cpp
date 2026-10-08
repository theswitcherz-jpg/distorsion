#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    const juce::Colour background (0xff050506);
    const juce::Colour panel (0xff0d0d0f);
    const juce::Colour silver (0xffd5d5d5);
    const juce::Colour muted (0xff77777d);
}

SPCTRLDAudioProcessorEditor::SPCTRLDAudioProcessorEditor(SPCTRLDAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(960, 620);
    setResizable(true, true);

    logo = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);

    title.setText("SPCTRL-D", juce::dontSendNotification);
    title.setFont(juce::Font(30.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, silver);
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);

    setupSectionLabel(distSection, "ASYMMETRIC DISTORTION");
    setupSectionLabel(compSection, "DYNAMICS / COMPRESSOR");
    setupSectionLabel(clipSection, "FINAL CLIPPER");

    setupSlider(drive, driveLabel, "DRIVE");
    setupSlider(asym, asymLabel, "ASYMMETRY");
    setupSlider(tone, toneLabel, "TONE");
    setupSlider(mix, mixLabel, "DIST MIX");

    setupSlider(compThresh, compThreshLabel, "THRESHOLD");
    setupSlider(compRatio, compRatioLabel, "RATIO");
    setupSlider(compAttack, compAttackLabel, "ATTACK");
    setupSlider(compRelease, compReleaseLabel, "RELEASE");
    setupSlider(compMakeup, compMakeupLabel, "MAKEUP");

    setupSlider(clip, clipLabel, "CLIP");
    setupSlider(clipMix, clipMixLabel, "CLIP MIX");
    setupSlider(output, outputLabel, "OUTPUT");

    const std::pair<juce::Slider*, const char*> mappings[] = {
        { &drive, "drive" }, { &asym, "asym" }, { &tone, "tone" }, { &mix, "mix" },
        { &compThresh, "compThresh" }, { &compRatio, "compRatio" }, { &compAttack, "compAttack" },
        { &compRelease, "compRelease" }, { &compMakeup, "compMakeup" },
        { &clip, "clip" }, { &clipMix, "clipMix" }, { &output, "output" }
    };

    for (auto [slider, id] : mappings)
        attachments.push_back(std::make_unique<SliderAttachment>(processor.parameters, id, *slider));
}

void SPCTRLDAudioProcessorEditor::setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 22);
    s.setColour(juce::Slider::rotarySliderFillColourId, silver);
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxTextColourId, silver);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff303036));
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(11.0f, juce::Font::bold));
    l.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(l);
}

void SPCTRLDAudioProcessorEditor::setupSectionLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(12.0f, juce::Font::bold));
    l.setColour(juce::Label::textColourId, silver);
    addAndMakeVisible(l);
}

void SPCTRLDAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);

    auto bounds = getLocalBounds().toFloat().reduced(14.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(bounds, 12.0f);
    g.setColour(juce::Colour(0xff2a2a2e));
    g.drawRoundedRectangle(bounds, 12.0f, 1.0f);

    if (logo.isValid())
    {
        auto logoArea = juce::Rectangle<float>(bounds.getCentreX() - 34.0f, 18.0f, 68.0f, 68.0f);
        g.drawImageWithin(logo, static_cast<int>(logoArea.getX()), static_cast<int>(logoArea.getY()),
                          static_cast<int>(logoArea.getWidth()), static_cast<int>(logoArea.getHeight()),
                          juce::RectanglePlacement::centred, false);
    }

    g.setColour(juce::Colour(0xff242428));
    g.fillRect(35, 104, getWidth() - 70, 1);
    g.fillRect(35, 318, getWidth() - 70, 1);
    g.fillRect(35, 512, getWidth() - 70, 1);
}

void SPCTRLDAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(34, 20);
    title.setBounds(area.removeFromTop(82));
    area.removeFromTop(2);

    distSection.setBounds(area.removeFromTop(24));
    auto dist = area.removeFromTop(170);
    const int dw = dist.getWidth() / 4;
    juce::Component* dSliders[] = { &drive, &asym, &tone, &mix };
    juce::Component* dLabels[] = { &driveLabel, &asymLabel, &toneLabel, &mixLabel };
    for (int i = 0; i < 4; ++i)
    {
        auto cell = dist.removeFromLeft(dw);
        dLabels[i]->setBounds(cell.removeFromTop(26));
        dSliders[i]->setBounds(cell.reduced(4));
    }

    compSection.setBounds(area.removeFromTop(24));
    auto comp = area.removeFromTop(170);
    const int cw = comp.getWidth() / 5;
    juce::Component* cSliders[] = { &compThresh, &compRatio, &compAttack, &compRelease, &compMakeup };
    juce::Component* cLabels[] = { &compThreshLabel, &compRatioLabel, &compAttackLabel, &compReleaseLabel, &compMakeupLabel };
    for (int i = 0; i < 5; ++i)
    {
        auto cell = comp.removeFromLeft(cw);
        cLabels[i]->setBounds(cell.removeFromTop(26));
        cSliders[i]->setBounds(cell.reduced(4));
    }

    clipSection.setBounds(area.removeFromTop(24));
    auto clipArea = area.removeFromTop(170);
    const int pw = clipArea.getWidth() / 3;
    juce::Component* pSliders[] = { &clip, &clipMix, &output };
    juce::Component* pLabels[] = { &clipLabel, &clipMixLabel, &outputLabel };
    for (int i = 0; i < 3; ++i)
    {
        auto cell = clipArea.removeFromLeft(pw);
        pLabels[i]->setBounds(cell.removeFromTop(26));
        pSliders[i]->setBounds(cell.reduced(4));
    }
}
