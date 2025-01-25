cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(MeloVST VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Bring in JUCE locally
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

CPMAddPackage(
    NAME libdatachannel
    GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
    GIT_TAG origin/master
)

CPMAddPackage(
    NAME ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG origin/master
)
CPMAddPackage(
    NAME r8brain-free-src
    GIT_REPOSITORY https://github.com/avaneev/r8brain-free-src.git
    GIT_TAG origin/master
)

CPMAddPackage(
    NAME libopus
    GIT_REPOSITORY https://github.com/xiph/opus.git
    GIT_TAG origin/main
)

include_directories(${libopus_SOURCE_DIR}/include)
include_directories(${r8brain-free-src_SOURCE_DIR})

add_library(
        datachannel_static INTERFACE IMPORTED
)
target_link_libraries(
        datachannel_static INTERFACE datachannel
)

# Make sure you include any new source files here
set(SourceFiles
        Source/Utils/StringUtils.h
        Source/Utils/AudioUtils.h
        Source/Utils/VectorUtils.h
        Source/Utils/FileUtils.h
        Source/Utils/Constants.h
        Source/Utils/JuceLocalStorage.h
        Source/Utils/json.hpp
        Source/Utils/AudioSettings.h
        Source/Debug/DebugAudioCallback.h
        Source/Debug/AudioAppPlayer.h
        Source/Debug/AudioAppPlayer.cpp
        Source/Debug/DebugRTPWrapper.h
        Source/MainApplication.cpp
        Source/MainApplication.h
        Source/CrashHandler.h
        Source/MainAudioProcessor.cpp
        Source/MainAudioProcessor.h
        Source/Gui/MainWindow.cpp
        Source/Gui/MainWindow.h
        Source/Gui/MainPageComponent.cpp
        Source/Gui/MainPageComponent.h
        Source/Gui/LoginPageComponent.cpp
        Source/Gui/LoginPageComponent.h
        Source/Api/ApiRoutes.h
        Source/Api/ApiService.cpp
        Source/Api/ApiService.h
        Source/Socket/WebSocketService.cpp
        Source/Socket/WebSocketService.h
        Source/Socket/SocketEvents.h
        Source/Socket/SocketRoutes.h
        Source/Api/AuthService.cpp
        Source/Api/AuthService.h
        Source/Rtc/WebRTCAudioService.h
        Source/Rtc/WebRTCAudioService.cpp
        Source/Rtc/WebRTCConnexionHandler.cpp
        Source/Rtc/WebRTCConnexionHandler.h
        Source/Rtc/ReconnectTimer.h
        Source/Rtc/OpusCodecWrapper.h
        Source/Rtc/RTPWrapper.h
        Source/Models/User.h
        Source/Models/Session.h
        Source/Models/Seller.h
        Source/Models/Artist.h
        Source/Models/UserContext.h
        Source/Events/EventListener.h
        Source/Events/EventManager.h
        Source/Events/Events.h
)

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME Melo
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Tap1
        PLUGIN_CODE Reg0
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "MeloVST"
)

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)


# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        r8brain-free-src
        opus
        datachannel
        ixwebsocket
        datachannel_static INTERFACE datachannel
        PUBLIC
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

add_subdirectory(Tests)