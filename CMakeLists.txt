cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(MeloVST VERSION 0.0.1)
option(IN_RECEIVING_MODE "Build Receiver VST plugin" OFF)
option(MELO_PLUGIN_NAME "Nom du plugin" "MeloVST")

message(STATUS "Le répertoire IN_RECEIVING_MODE est : $ENV{IN_RECEIVING_MODE}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(USE_OPEN_SSL ON)
set(USE_TLS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if ($ENV{IN_RECEIVING_MODE})
    set(MELO_PLUGIN_NAME "MeloVSTReceive")
    add_definitions(-DIN_RECEIVING_MODE)
    set(IN_RECEIVING_MODE ON)
else ()
    set(MELO_PLUGIN_NAME "MeloVSTSend")
endif ()

message(STATUS "Building ${MELO_PLUGIN_NAME}")
message(STATUS "PROJECT_NAME ${PROJECT_NAME}")
message(STATUS "IN_RECEIVING_MODE ${IN_RECEIVING_MODE}")

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
        GIT_TAG 8.0.6
)

CPMAddPackage(
        NAME ssl
        GIT_REPOSITORY https://github.com/jimmy-park/openssl-cmake.git
        GIT_TAG 3.4.0
)

CPMAddPackage(
        NAME libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG v0.22.4
        OPTIONS
        "PREFER_SYSTEM_LIB=0"
)

CPMAddPackage(
        NAME ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG origin/master
)
message(STATUS "OpenSSL SSL library: ${OPENSSL_SSL_LIBRARY}")
message(STATUS "OpenSSL CRYPTO library: ${OPENSSL_CRYPTO_LIBRARY}")
message(STATUS "ixwebsocket SOURCE_DIR: ${ixwebsocket_SOURCE_DIR}")
message(STATUS "ixwebsocket BUILD DIR: ${ixwebsocket_SOURCE_DIR}")
find_package(OpenSSL REQUIRED)
target_link_libraries(ixwebsocket PRIVATE OpenSSL::SSL OpenSSL::Crypto)

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

include_directories(${ixwebsocket_SOURCE_DIR})
include_directories(${libopus_SOURCE_DIR})
include_directories(${r8brain_SOURCE_DIR})

add_library(r8brain STATIC)
target_sources(r8brain PRIVATE "${r8brain_SOURCE_DIR}/r8bbase.cpp")
add_library(
        datachannel_static INTERFACE IMPORTED
)
target_link_libraries(
        datachannel_static INTERFACE datachannel
)


# Make sure you include any new source files here
set(SourceFiles
        Source/MainApplication.cpp
        Source/MainApplication.h
        Source/MainAudioProcessor.cpp
        Source/MainAudioProcessor.h
        Source/Config.h
        Source/AudioSettings.h

        Source/Utils/FileUtils.h
        Source/Utils/StringUtils.h
        Source/Utils/VectorUtils.h

        Source/ThirdParty/json.hpp

        Source/Rtc/WebRTCConnexionState.h
        Source/Rtc/WebRTCConnexionState.cpp
        Source/RtcReceiver/WebRTCAudioReceiverService.h
        Source/RtcReceiver/WebRTCAudioReceiverService.cpp
        Source/RtcReceiver/WebRTCReceiverConnexionHandler.cpp
        Source/RtcReceiver/WebRTCReceiverConnexionHandler.h
        Source/RtcSender/WebRTCAudioSenderService.h
        Source/RtcSender/WebRTCAudioSenderService.cpp
        Source/RtcSender/WebRTCSenderConnexionHandler.cpp
        Source/RtcSender/WebRTCSenderConnexionHandler.h

        Source/Models/User.h
        Source/Models/Session.h
        Source/Models/Seller.h
        Source/Models/Artist.h
        Source/Models/UserContext.h

        Source/Gui/MainWindow.cpp
        Source/Gui/MainWindow.h
        Source/Gui/MainPageComponent.cpp
        Source/Gui/MainPageComponent.h
        Source/Gui/LoginPageComponent.cpp
        Source/Gui/LoginPageComponent.h

        Source/Debug/DebugAudioCallback.h
        Source/Debug/DebugAudioAppPlayer.h
        Source/Debug/DebugRTPWrapper.h

        Source/Common/CircularBuffer.h
        Source/Common/CrashHandler.h
        Source/Common/EventListener.h
        Source/Common/EventManager.h
        Source/Common/Events.h
        Source/Common/JuceLocalStorage.h
        Source/Common/OpusCodecWrapper.h
        Source/Common/OpusFileHandler.h
        Source/Common/ReconnectTimer.h
        Source/Common/ResamplerWrapper.h
        Source/Common/RTPWrapper.h
        Source/Common/WavFileHandler.h

        Source/Api/ApiRoutes.h
        Source/Api/ApiService.cpp
        Source/Api/ApiService.h
        Source/Api/AuthService.cpp
        Source/Api/AuthService.h
        Source/Api/WebSocketService.cpp
        Source/Api/WebSocketService.h
        Source/Api/SocketEvents.h
        Source/Api/SocketRoutes.h
)

message(STATUS "PRODUCT_NAME: ${MELO_PLUGIN_NAME}")
juce_add_plugin(${MELO_PLUGIN_NAME}
        COMPANY_NAME melo
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Tap1
        PLUGIN_CODE MEL0
        FORMATS VST3 AU Standalone
)
# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${MELO_PLUGIN_NAME} PRIVATE ${SourceFiles})

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${MELO_PLUGIN_NAME}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ADD_R8BRAIN=1
        JUCE_BUILD_EXTRAS=ON
        PREFER_SYSTEM_LIB=1
        JUCE_COPY_PLUGIN_AFTER_BUILD=1

)

# JUCE libraries to bring into our project
target_link_libraries(${MELO_PLUGIN_NAME}
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

get_target_property(IXWEB_INCLUDE ixwebsocket INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "IXWEB_INCLUDE: ${IXWEB_INCLUDE}")

if (IXWEB_INCLUDE)
    # Sépare la chaîne en une liste d'arguments
    separate_arguments(IXWEB_INCLUDE_LIST UNIX_COMMAND "${IXWEB_INCLUDE}")
    set(new_includes "")
    foreach(item IN LISTS IXWEB_INCLUDE_LIST)
        message(STATUS "item: ${item}")
        # Vérifie si item contient le chemin à supprimer
        string(FIND "${item}" "${ixwebsocket_SOURCE_DIR}" pos)
        if (pos EQUAL -1)
            list(APPEND new_includes "${item}")
        else()
            message(STATUS "Removing: ${item}")
        endif()
    endforeach()
    message(STATUS "new_includes: ${new_includes}")
    set_target_properties(ixwebsocket PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${new_includes}")
endif()

add_subdirectory(Tests)