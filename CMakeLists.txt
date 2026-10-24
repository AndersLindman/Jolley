cmake_minimum_required(VERSION 3.22)

project(Jolley VERSION 0.1.0)

# Default to Release if the user didn’t specify a build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type" FORCE)
endif()

include(FetchContent)

# ----------------------------------------------------------------------
# JUCE
# ----------------------------------------------------------------------
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.16-security
)
FetchContent_MakeAvailable(JUCE)

# ----------------------------------------------------------------------
# WebView2 (Windows) – keep the location that already works for you
# ----------------------------------------------------------------------
if (WIN32)
    set(JUCE_WEBVIEW2_PACKAGE_LOCATION
        "$ENV{USERPROFILE}/AppData/Local/PackageManagement/NuGet/Packages"
        CACHE PATH "Path to the WebView2 NuGet package directory"
    )
endif()

add_custom_target(BuildFrontend DEPENDS "${UI_ZIP_PATH}")

# ----------------------------------------------------------------------
# Embedded web UI (zip)
# ----------------------------------------------------------------------
# Note: BinaryData::getNamedResource() uses the original filename string 
# ("jolley-gui.zip"), so no need to check the generated C++ variable names.
juce_add_binary_data(JolleyAssets
    SOURCES
        "Resources/jolley-gui.zip"
)

# ----------------------------------------------------------------------
# Plugin definition
# ----------------------------------------------------------------------
juce_add_plugin(Jolley
    COMPANY_NAME               "Achrono"
    IS_SYNTH                   TRUE
    NEEDS_MIDI_INPUT           TRUE
    IS_MIDI_EFFECT             FALSE
    PLUGIN_MANUFACTURER_CODE   "Achr"
    PLUGIN_CODE                "Joll"
    # Added AU for macOS Logic/GarageBand support. Remove if not needed.
    FORMATS                    VST3 AU Standalone 
    PRODUCT_NAME               "Jolley"
    NEEDS_WEBVIEW2             TRUE          # required for WebBrowserComponent on Windows
)

# ----------------------------------------------------------------------
# Sources
# ----------------------------------------------------------------------
target_sources(Jolley
    PRIVATE
        Source/JolleyBrowserComponent.h
        Source/ParamIds.h
        Source/PluginEditor.h
        Source/PluginEditor.cpp
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp

        Source/dsp/ADSREnvelope.h
        Source/dsp/ADSREnvelope.cpp
        Source/dsp/DiodeFilter.h
        Source/dsp/DiodeFilter.cpp
        Source/dsp/DSFOscillator.h
        Source/dsp/DSFOscillator.cpp
        Source/dsp/DSPUtils.h
        Source/dsp/LadderFilter.h
        Source/dsp/LadderFilter.cpp
        Source/dsp/LFO.h
        Source/dsp/MultimodeFilter.h
        Source/dsp/MultimodeFilter.cpp
        Source/dsp/PitchEnvelope.h
        Source/dsp/TubeDrive.h
        Source/dsp/ZDFfilter.h
        Source/dsp/ZDFfilter.cpp

        Source/dsp/synth/FMProcessor.h
        Source/dsp/synth/FMProcessor.cpp
        Source/dsp/synth/SynthEngine.h
        Source/dsp/synth/SynthEngine.cpp
        Source/dsp/synth/SynthVoice.h
        Source/dsp/synth/SynthVoice.cpp
        Source/dsp/synth/Unison.h
        Source/dsp/synth/Unison.cpp
)

# ----------------------------------------------------------------------
# Compile definitions
# ----------------------------------------------------------------------
target_compile_definitions(Jolley
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1   # matches the official demo
)

# ----------------------------------------------------------------------
# JUCE header + linking
# ----------------------------------------------------------------------
juce_generate_juce_header(Jolley)

target_link_libraries(Jolley
    PRIVATE
        JolleyAssets                    # the zip
        juce::juce_audio_basics
        juce::juce_audio_processors     # Explicitly linked for APVTS
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra            # needed for WebBrowserComponent

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# ----------------------------------------------------------------------
# Release performance flags (Target-specific, safe for distribution)
# ----------------------------------------------------------------------
# We use target_compile_options instead of global CMAKE_CXX_FLAGS to avoid 
# polluting JUCE's own build flags.
if(MSVC)
    target_compile_options(Jolley PRIVATE $<$<CONFIG:Release>:/O2 /Oi /GL /arch:AVX2>)
    target_link_options(Jolley PRIVATE $<$<CONFIG:Release>:/LTCG>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Removed -march=native to prevent crashes on user machines with different CPUs!
    target_compile_options(Jolley PRIVATE $<$<CONFIG:Release>:-O3 -flto>)
    target_link_options(Jolley PRIVATE $<$<CONFIG:Release>:-flto>)
endif()