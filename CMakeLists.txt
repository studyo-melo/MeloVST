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
        GIT_TAG v0.22.4
        OPTIONS "PREFER_SYSTEM_LIB=0"
)

CPMAddPackage(
        NAME ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG origin/master
)
CPMAddPackage(
        NAME r8brain
        GIT_REPOSITORY https://github.com/avaneev/r8brain-free-src.git
        GIT_TAG origin/master
)

CPMAddPackage(
        NAME libopus
        GIT_REPOSITORY https://github.com/xiph/opus.git
        GIT_TAG v1.5.2
)

include_directories(${libopus_SOURCE_DIR}/include)
include_directories(${r8brain_SOURCE_DIR})
add_library(r8brain STATIC)
target_sources(r8brain PRIVATE "${r8brain_SOURCE_DIR}/r8bbase.cpp")
add_library(
        datachannel_static INTERFACE IMPORTED
)
target_link_libraries(
        datachannel_static INTERFACE datachannel
)

include_directories(
        deps/aoo/lib
        deps/aoo/deps
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
        Source/Utils/ResamplerWrapper.h
        Source/Debug/DebugAudioCallback.h
        Source/Debug/CircularBuffer.h
        Source/Debug/DebugRTPWrapper.h
        Source/Files/WavFileHandler.h
        Source/Files/OpusFileHandler.h
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
        Source/Encoders/OpusCodecWrapper.h
        Source/Encoders/OpusEncoderJuce.h
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
set(AOOSourceFiles
        deps/aoo/lib/src/SLIP.hpp
        deps/aoo/lib/src/client.cpp
        deps/aoo/lib/src/client.hpp
        deps/aoo/lib/src/codec_opus.cpp
        deps/aoo/lib/src/codec_pcm.cpp
        deps/aoo/lib/src/common.cpp
        deps/aoo/lib/src/common.hpp
        deps/aoo/lib/src/lockfree.hpp
        deps/aoo/lib/src/net_utils.cpp
        deps/aoo/lib/src/net_utils.hpp
        deps/aoo/lib/src/server.cpp
        deps/aoo/lib/src/server.hpp
        deps/aoo/lib/src/sink.cpp
        deps/aoo/lib/src/sink.hpp
        deps/aoo/lib/src/source.cpp
        deps/aoo/lib/src/source.hpp
        deps/aoo/lib/src/sync.cpp
        deps/aoo/lib/src/sync.hpp
        deps/aoo/lib/src/time.cpp
        deps/aoo/lib/src/time.hpp
        deps/aoo/lib/src/time_dll.hpp
        deps/aoo/lib/aoo/aoo.h
        deps/aoo/lib/aoo/aoo.hpp
        deps/aoo/lib/aoo/aoo_net.h
        deps/aoo/lib/aoo/aoo_net.hpp
        deps/aoo/lib/aoo/aoo_opus.h
        deps/aoo/lib/aoo/aoo_pcm.h
        deps/aoo/lib/aoo/aoo_types.h
        deps/aoo/lib/aoo/aoo_utils.hpp

        deps/aoo/deps/md5/md5.c
        deps/aoo/deps/md5/md5.h
        deps/aoo/deps/oscpack/osc/OscOutboundPacketStream.cpp
        deps/aoo/deps/oscpack/osc/OscPrintReceivedElements.cpp
        deps/aoo/deps/oscpack/osc/OscReceivedElements.cpp
        deps/aoo/deps/oscpack/osc/OscTypes.cpp
        deps/aoo/deps/oscpack/osc/MessageMappingOscPacketListener.h
        deps/aoo/deps/oscpack/osc/OscException.h
        deps/aoo/deps/oscpack/osc/OscHostEndianness.h
        deps/aoo/deps/oscpack/osc/OscOutboundPacketStream.h
        deps/aoo/deps/oscpack/osc/OscPacketListener.h
        deps/aoo/deps/oscpack/osc/OscPrintReceivedElements.h
        deps/aoo/deps/oscpack/osc/OscReceivedElements.h
        deps/aoo/deps/oscpack/osc/OscTypes.h
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
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles} ${AOOSourceFiles})

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ADD_R8BRAIN=1
        PREFER_SYSTEM_LIB=1
)


# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        opus
        datachannel
        r8brain
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