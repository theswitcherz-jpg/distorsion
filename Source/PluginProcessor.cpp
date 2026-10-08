#include "PluginProcessor.h"
#include "PluginEditor.h"

SPCTRLDAudioProcessor::SPCTRLDAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SPCTRLDAudioProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", Range(0.0f, 36.0f, 0.01f), 12.0f, " dB"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("asym", "Asymmetry", Range(-1.0f, 1.0f, 0.001f), 0.35f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("tone", "Tone", Range(0.0f, 1.0f, 0.001f), 0.60f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Dist Mix", Range(0.0f, 1.0f, 0.001f), 1.0f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("compThresh", "Comp Threshold", Range(-48.0f, 0.0f, 0.01f), -18.0f, " dB"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("compRatio", "Comp Ratio", Range(1.0f, 20.0f, 0.01f), 4.0f, ":1"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("compAttack", "Comp Attack", Range(0.1f, 100.0f, 0.01f), 8.0f, " ms"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("compRelease", "Comp Release", Range(10.0f, 500.0f, 0.1f), 100.0f, " ms"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("compMakeup", "Comp Makeup", Range(0.0f, 18.0f, 0.01f), 3.0f, " dB"));

    p.push_back(std::make_unique<juce::AudioParameterFloat>("clip", "Clip", Range(0.0f, 12.0f, 0.01f), 3.0f, " dB"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("clipMix", "Clip Mix", Range(0.0f, 1.0f, 0.001f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", Range(-24.0f, 6.0f, 0.01f), -3.0f, " dB"));

    return { p.begin(), p.end() };
}

void SPCTRLDAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    compressor.prepare(spec);
    compressor.reset();

    toneState.fill(0.0f);
    dcX1.fill(0.0f);
    dcY1.fill(0.0f);
}

void SPCTRLDAudioProcessor::releaseResources()
{
    compressor.reset();
}

bool SPCTRLDAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getChannelSet(true, 0);
    const auto mainOut = layouts.getChannelSet(false, 0);
    return (mainIn == juce::AudioChannelSet::mono() || mainIn == juce::AudioChannelSet::stereo())
        && mainIn == mainOut;
}

float SPCTRLDAudioProcessor::processAsymmetric(float x, float drive, float asym) const noexcept
{
    const float posDrive = juce::jmax(0.05f, drive * (1.0f + asym));
    const float negDrive = juce::jmax(0.05f, drive * (1.0f - asym));

    if (x >= 0.0f)
        return std::tanh(x * posDrive);

    return std::tanh(x * negDrive);
}

void SPCTRLDAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    if (channels == 0)
        return;

    const float driveDb = parameters.getRawParameterValue("drive")->load();
    const float asym = parameters.getRawParameterValue("asym")->load();
    const float tone = parameters.getRawParameterValue("tone")->load();
    const float mix = parameters.getRawParameterValue("mix")->load();
    const float compThreshold = parameters.getRawParameterValue("compThresh")->load();
    const float compRatio = parameters.getRawParameterValue("compRatio")->load();
    const float compAttack = parameters.getRawParameterValue("compAttack")->load();
    const float compRelease = parameters.getRawParameterValue("compRelease")->load();
    const float compMakeup = parameters.getRawParameterValue("compMakeup")->load();
    const float clipDb = parameters.getRawParameterValue("clip")->load();
    const float clipMix = parameters.getRawParameterValue("clipMix")->load();
    const float outputDb = parameters.getRawParameterValue("output")->load();

    compressor.setThreshold(compThreshold);
    compressor.setRatio(compRatio);
    compressor.setAttack(compAttack);
    compressor.setRelease(compRelease);

    const float drive = juce::Decibels::decibelsToGain(driveDb);
    const float output = juce::Decibels::decibelsToGain(outputDb);
    const float makeup = juce::Decibels::decibelsToGain(compMakeup);
    const float clipThreshold = juce::Decibels::decibelsToGain(-clipDb);

    const float cutoff = 500.0f + tone * 17000.0f;
    const float toneCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / static_cast<float>(currentSampleRate));

    // Distortion / tone / DC cleanup.
    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        const float dryGain = 1.0f - mix;

        for (int i = 0; i < samples; ++i)
        {
            const float dry = data[i];
            float x = dry * drive;
            x = processAsymmetric(x, 1.0f, asym);

            // Tone is a high-frequency tilt controlled by a simple one-pole low-pass blend.
            const float lp = toneState[static_cast<size_t>(ch)] =
                toneCoeff * toneState[static_cast<size_t>(ch)] + (1.0f - toneCoeff) * x;
            const float toned = lp * (1.0f - tone) + x * tone;

            // Remove the DC offset introduced by asymmetry without changing the audible shape much.
            const float hpX = toned - dcX1[static_cast<size_t>(ch)] + 0.995f * dcY1[static_cast<size_t>(ch)];
            dcX1[static_cast<size_t>(ch)] = toned;
            dcY1[static_cast<size_t>(ch)] = hpX;

            data[i] = dry * dryGain + hpX * mix;
        }
    }

    // Glue / punch compressor.
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);

    // Makeup and final hard clipper. Clip has a dry/wet control so it can be used as a parallel peak shaper.
    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < samples; ++i)
        {
            const float preClip = data[i] * makeup;
            const float clipped = juce::jlimit(-1.0f, 1.0f, preClip / juce::jmax(0.05f, clipThreshold))
                                 * juce::jmax(0.05f, clipThreshold);
            data[i] = (preClip * (1.0f - clipMix) + clipped * clipMix) * output;
        }
    }
}

void SPCTRLDAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SPCTRLDAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* SPCTRLDAudioProcessor::createEditor()
{
    return new SPCTRLDAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SPCTRLDAudioProcessor();
}
