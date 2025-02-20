cmake_minimum_required(VERSION 3.24)

project(MeloVST VERSION 0.0.1)

####################################################################################################
### VARIABLES ###
####################################################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(USE_OPEN_SSL ON)
set(USE_TLS ON)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application: Silver Roves Studio (9W2L5R73RN)")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "9W2L5R73RN")
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


####################################################################################################
### INSTALL ###
####################################################################################################

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
        NAME juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
)

CPMAddPackage(
        NAME libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG v0.22.5
#        OPTIONS
#        "PREFER_SYSTEM_LIB=OFF"
)

CPMAddPackage(
        NAME ssl
        GIT_REPOSITORY https://github.com/jimmy-park/openssl-cmake.git
        GIT_TAG 3.4.0
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

####################################################################################################
### SOURCES ###
####################################################################################################

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
        Source/Common/OpusEncoderWrapper.h
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
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})


####################################################################################################
### Plugins ###
####################################################################################################
#juce_set_aax_sdk_path("/Applications/Avid/AAX_SDKs/AAX_SDK_2p3p2")
#juce_set_vst3_sdk_path("/Applications/VST_SDK/VST3_SDK")
#juce_set_vst2_sdk_path("/Applications/VST_SDK/VST_SDK")

juce_add_plugin(MeloVSTSend
        PRODUCT_NAME "MeloVSTSend"
        VERSION 0.0.1
        BUILD_VERSION 0.0.1
        BUNDLE_ID com.melo.MeloVSTSend
        COMPANY_NAME melo
        COMPANY_WEBSITE "https://studio-melo.com"
        COMPANY_EMAIL "contact@studio-melo.com"
        PLUGIN_NAME "MeloVSTSend"
        PLUGIN_MANUFACTURER_CODE MelS
        PLUGIN_CODE MEL0
        DESCRIPTION "Melo Plugin to send the music to the artist"

        VST3_CATEGORIES "Network"
        COPY_PLUGIN_AFTER_BUILD TRUE
        HARDENED_RUNTIME_ENABLED TRUE
        FORMATS VST3 AU Standalone
        VST3_AUTO_MANIFEST FALSE

        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
)

juce_add_plugin(MeloVSTReceive
        PRODUCT_NAME "MeloVSTReceive"
        VERSION 0.0.1
        BUILD_VERSION 0.0.1
        BUNDLE_ID com.melo.MeloVSTReceive
        COMPANY_NAME melo
        COMPANY_WEBSITE "https://studio-melo.com"
        COMPANY_EMAIL "contact@studio-melo.com"
        PLUGIN_NAME "MeloVSTReceive"
        PLUGIN_MANUFACTURER_CODE MelR
        PLUGIN_CODE MEL0
        DESCRIPTION "Melo Plugin to receive the voice of the artist"

        VST3_CATEGORIES "Network"
        COPY_PLUGIN_AFTER_BUILD TRUE
        HARDENED_RUNTIME_ENABLED TRUE
        FORMATS VST3 AU Standalone
        VST3_AUTO_MANIFEST FALSE

        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(MeloVSTSend PRIVATE ${SourceFiles})
target_sources(MeloVSTReceive PRIVATE ${SourceFiles})

target_compile_definitions(MeloVSTSend
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_ADD_R8BRAIN=1
        JUCE_BUILD_EXTRAS=OFF
)

target_compile_definitions(MeloVSTReceive
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_ADD_R8BRAIN=1
        JUCE_BUILD_EXTRAS=OFF
        IN_RECEIVING_MODE=1
)


set_target_properties(MeloVSTSend PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE}"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY}"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}"
)
set_target_properties(MeloVSTReceive PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE}"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY}"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}"
)

target_link_libraries(MeloVSTSend
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

target_link_libraries(MeloVSTReceive
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

#juce_enable_vst3_manifest_step(MeloVSTSend)
#juce_enable_vst3_manifest_step(MeloVSTReceive)
#juce_enable_copy_plugin_step(MeloVSTSend)
#juce_enable_copy_plugin_step(MeloVSTReceive)
####################################################################################################
### IXWebsocket headers ###
####################################################################################################
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