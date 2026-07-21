cmake_minimum_required(VERSION 3.22)

project(AutoDuck VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

set(AUTODUCK_FORMATS VST3 Standalone)
if(APPLE)
    list(PREPEND AUTODUCK_FORMATS AU)
endif()

juce_add_plugin(AutoDuck
    COMPANY_NAME "Joe Speaker"
    BUNDLE_ID com.joespeaker.autoduck
    PLUGIN_MANUFACTURER_CODE Jspk
    PLUGIN_CODE Adck
    FORMATS ${AUTODUCK_FORMATS}
    PRODUCT_NAME "Auto-Duck"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Dynamics
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(AutoDuck
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(AutoDuck
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(AutoDuck
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
