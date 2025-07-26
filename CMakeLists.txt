cmake_minimum_required(VERSION 3.30)

project(EQlibrium VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(extern/JUCE)

juce_add_plugin(EQlibrium
        VERSION "0.1"
        # ICON_BIG ...
        ICON_SMALL "images/logo2.png"
        COMPANY_NAME "Boostmeup GmbH"
        IS_SYNTH FALSE
        MICROPHONE_PERMISSION_ENABLED TRUE
        #BACKGROUND_AUDIO_ENABLED TRUE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Bsmp
        PLUGIN_CODE Eqlm
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "EQlibrium")

juce_generate_juce_header(EQlibrium)

target_sources(EQlibrium
        PRIVATE
        src/main/PluginEditor.cpp
        src/main/PluginProcessor.cpp
        src/graph/ResponseCurveComponent.cpp
        src/graph/PathProducer.cpp
        src/inputs/RotarySlider.cpp
        src/level_meter/LevelMeter.cpp
        src/inputs/LoudnessSlider.cpp
        src/inputs/ChannelButtons.cpp
        src/inputs/NormalImageButtons.cpp
        src/image/EQImages.cpp
        src/graph/WaveForm.cpp
        src/inputs/PlayButton.cpp
        src/inputs/GlowImageButtons.cpp)

target_compile_definitions(EQlibrium
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        DONT_SET_USING_JUCE_NAMESPACE
        JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(EQlibrium
        PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_events
        juce::juce_dsp
        juce::juce_audio_formats
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

set(DEBUG_PATH_STANDALONE "${CMAKE_CURRENT_BINARY_DIR}/EQlibrium_artefacts/Debug/Standalone")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images
        DESTINATION ${DEBUG_PATH_STANDALONE}/)
make_directory(${DEBUG_PATH_STANDALONE}/records)
set(DEBUG_PATH_VST3 "${CMAKE_CURRENT_BINARY_DIR}/EQlibrium_artefacts/Debug/VST3")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images
        DESTINATION ${DEBUG_PATH_VST3}/)
make_directory(${DEBUG_PATH_VST3}/records)
set(RELEASE_PATH_STANDALONE "${CMAKE_CURRENT_BINARY_DIR}/EQlibrium_artefacts/Release/Standalone")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images
        DESTINATION ${RELEASE_PATH_STANDALONE}/)
make_directory(${RELEASE_PATH_STANDALONE}/records)
set(RELEASE_PATH_VST3 "${CMAKE_CURRENT_BINARY_DIR}/EQlibrium_artefacts/Release/VST3")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images
        DESTINATION ${RELEASE_PATH_VST3}/)
make_directory(${RELEASE_PATH_VST3}/records)