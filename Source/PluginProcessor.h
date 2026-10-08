#pragma once
#include <JuceHeader.h>

class SPCTRLDAudioProcessor : public juce::AudioProcessor
{
public:
    SPCTRLDAudioProcessor();
    ~SPCTRLDAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SPCTRL-D"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::dsp::Compressor<float> compressor;
    std::array<float, 2> toneState { 0.0f, 0.0f };
    std::array<float, 2> dcX1 { 0.0f, 0.0f };
    std::array<float, 2> dcY1 { 0.0f, 0.0f };
    double currentSampleRate = 44100.0;

    float processAsymmetric(float x, float drive, float asym) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPCTRLDAudioProcessor)
};
