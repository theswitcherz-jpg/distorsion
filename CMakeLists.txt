cmake_minimum_required(VERSION 3.22)
project(SPCTRL_D VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_binary_data(SPCTRLDBinaryData
    SOURCES
        Assets/logo.png
)

juce_add_plugin(SPCTRL_D
    COMPANY_NAME "Boris Audio"
    BUNDLE_ID "com.borisaudio.spctrld"
    PLUGIN_MANUFACTURER_CODE BrAu
    PLUGIN_CODE SpD1
    FORMATS VST3
    PRODUCT_NAME "SPCTRL-D"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(SPCTRL_D PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(SPCTRL_D PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SPCTRL_D PRIVATE
    SPCTRLDBinaryData
    juce::juce_audio_utils
    juce::juce_dsp
)
