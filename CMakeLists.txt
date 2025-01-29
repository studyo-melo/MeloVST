cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(MeloVST VERSION 0.0.1)
cmake_policy(SET CMP0002 NEW)


####################################################################################################
# CMake Configuration
####################################################################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -arch arm64 -lssl -lcrypto -lspeex")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
set(CMAKE_MACOSX_RPATH 1)

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

####################################################################################################
# CPM Modules
####################################################################################################
include(${CPM_DOWNLOAD_LOCATION})

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
    NAME r8brain
    GIT_REPOSITORY https://github.com/avaneev/r8brain-free-src.git
    GIT_TAG origin/master
)

CPMAddPackage(
    NAME libopus
    GIT_REPOSITORY https://github.com/xiph/opus.git
    GIT_TAG v1.5.2
)
CPMAddPackage(
    NAME speex
    GIT_REPOSITORY https://github.com/xiph/speex.git
    GIT_TAG Speex-1.2.1
)

CPMAddPackage(
        NAME metaRTC
        GIT_REPOSITORY git@github.com:metartc/metaRTC.git
        GIT_TAG v7.0-b1
)

CPMAddPackage(
        NAME usrsctp
        GIT_REPOSITORY https://github.com/sctplab/usrsctp.git
        GIT_TAG 0.9.5.0
)

CPMAddPackage(
        NAME libsrtp
        GIT_REPOSITORY https://github.com/cisco/libsrtp.git
        GIT_TAG  v2.6.0
)

####################################################################################################
# Include Directories
####################################################################################################

find_package(OpenSSL REQUIRED)
find_package(Speex REQUIRED)
#find_package(Usrsrtp REQUIRED)
#find_package(LibSRTP REQUIRED)


add_subdirectory(${metaRTC_SOURCE_DIR}/libmetartccore7 ${metaRTC_SOURCE_DIR}/libmetartc7)
include_directories(
        ${metaRTC_SOURCE_DIR}/include
        ${metaRTC_SOURCE_DIR}/thirdparty/include
        ${metaRTC_SOURCE_DIR}/thirdparty/user_include
        ${metaRTC_SOURCE_DIR}/libmetartc7
        ${metaRTC_SOURCE_DIR}/libmetartccore7
)
include_directories(${speex_SOURCE_DIR}/include)
include_directories(${libsrtp_SOURCE_DIR}/include)
include_directories(${usrsctp_SOURCE_DIR}/include)
include_directories(/opt/homebrew/opt/openssl/include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${libopus_SOURCE_DIR}/include)
include_directories(${r8brain_SOURCE_DIR})
include_directories(${datachannel_SOURCE_DIR})


link_directories(/opt/homebrew/opt/openssl/lib)

add_library(r8brain STATIC)
target_sources(r8brain PRIVATE "${r8brain_SOURCE_DIR}/r8bbase.cpp")

add_library(datachannel_static INTERFACE IMPORTED)
target_link_libraries(datachannel_static INTERFACE datachannel)

####################################################################################################
# Source Files
####################################################################################################
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
        JUCE_ADD_R8BRAIN=1
)

####################################################################################################
# Link Libraries
####################################################################################################

target_link_libraries(metartccore7 ${OPENSSL_LIBRARIES} ${speex_LIBRARIES} ${libopus_LIBRARIES} ${libsrtp_LIBRARIES} ${usrsctp_LIBRARIES})
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        opus
        datachannel
        r8brain
        ixwebsocket
        speex
        ssl
        libsrtp
        usrsctp
        crypto
        metaRTC
        metartccore7
        metartc7
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
