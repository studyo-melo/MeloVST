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
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -arch arm64 -lssl -lcrypto -lspeex -lusrsctp -lsrtp2 -lffmpeg")
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
    NAME ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
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
        NAME usrsctp
        GIT_REPOSITORY https://github.com/sctplab/usrsctp.git
        GIT_TAG 0.9.5.0
)

CPMAddPackage(
        NAME libsrtp
        GIT_REPOSITORY https://github.com/cisco/libsrtp.git
        GIT_TAG v2.6.0
        OPTIONS
        "ENABLE_STATIC ON" # Force la compilation de la version statique
        "ENABLE_SHARED ON" # Active la compilation d'une version dynamique (.so ou .dylib)
)
CPMAddPackage(
        NAME speexdsp
        GIT_REPOSITORY https://github.com/xiph/speexdsp.git
        GIT_TAG SpeexDSP-1.2.1
        OPTIONS
        "ENABLE_STATIC ON" # Force la compilation de la version statique
        "ENABLE_SHARED ON" # Active la compilation d'une version dynamique (.so ou .dylib)
)
#
file(GLOB SPEEX_SOURCES ${speexdsp_SOURCE_DIR}/libspeexdsp/*.c)
add_library(speexdsp STATIC ${SPEEX_SOURCES})
target_include_directories(speexdsp PUBLIC ${speexdsp_SOURCE_DIR}/include)
#
## Exécute le script `autogen.sh` pour générer les fichiers de configuration
execute_process(
        COMMAND ${speexdsp_SOURCE_DIR}/autogen.sh ./libspeexdsp
        WORKING_DIRECTORY ${speexdsp_SOURCE_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        COMMAND_ERROR_IS_FATAL ANY
)
#
execute_process(
        COMMAND ${speexdsp_SOURCE_DIR}/configure --prefix=${CMAKE_BINARY_DIR}/speexdsp
        WORKING_DIRECTORY ${speexdsp_SOURCE_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        COMMAND_ERROR_IS_FATAL ANY
)
#
execute_process(
        COMMAND make
        WORKING_DIRECTORY ${speexdsp_SOURCE_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        COMMAND_ERROR_IS_FATAL ANY
)
#
execute_process(
        COMMAND make install
        WORKING_DIRECTORY ${speexdsp_SOURCE_DIR}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        COMMAND_ERROR_IS_FATAL ANY
)
add_definitions(-DFLOATING_POINT -DEXPORT= -DUSE_SMALLFT)
####################################################################################################
# Include Directories
####################################################################################################
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(OPENH264_LIBRARY openh264)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavcodec libavformat libavutil libswscale libswresample)

link_directories(/opt/homebrew/opt/openssl/lib)
link_directories(/opt/homebrew/opt/ffmpeg/lib)

add_subdirectory(${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/libmetartccore7)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/libmetartc7)
include_directories(
        ${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/include
        ${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/thirdparty/include
        ${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/thirdparty/user_include
        ${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/libmetartc7
        ${CMAKE_SOURCE_DIR}/Source/ThirdParty/metaRTC/libmetartccore7
)
include_directories(/opt/homebrew/opt/openssl/include)
include_directories(/opt/homebrew/opt/ffmpeg/include)
include_directories(${libopus_SOURCE_DIR}/include)
include_directories(${speex_SOURCE_DIR}/include)
include_directories(${libsrtp_SOURCE_DIR}/include)
include_directories(${usrsctp_SOURCE_DIR}/usrsctplib)
include_directories(${OPENSSL_INCLUDE_DIR})

message(STATUS "Compilation en cours...")
message(STATUS "ffmpeg_INCLUDE_DIR: ${ffmpeg_INCLUDE_DIR}")
message(STATUS "ffmpeg_LIBRARIES: ${ffmpeg_LIBRARIES}")
message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message(STATUS "SPEEX_SOURCE_DIR: ${speex_SOURCE_DIR}")
message(STATUS "SPEEX_BINARY_DIR: ${speex_BINARY_DIR}")
message(STATUS "libsrtp_SOURCE_DIR: ${libsrtp_SOURCE_DIR}")
message(STATUS "libsrtp_BINARY_DIR: ${libsrtp_BINARY_DIR}")
message(STATUS "USRSCTP_SOURCE_DIR: ${usrsctp_SOURCE_DIR}")
message(STATUS "USRSCTP_BINARY_DIR: ${usrsctp_BINARY_DIR}")
message(STATUS "libmetartc7_SOURCE_DIR: ${libmetartc7_SOURCE_DIR}")
message(STATUS "libmetartccore7_SOURCE_DIR: ${libmetartccore7_SOURCE_DIR}")
message(STATUS "AVFoundation_LIBRARY: ${AVFOUNDATION_LIBRARY}")
message(STATUS "AudioToolbox_LIBRARY: ${AUDIOTOOLBOX_LIBRARY}")
message(STATUS "OPENH264_LIBRARY: ${OPENH264_LIBRARY}")

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
        Source/Utils/RTCUtils.h
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
        Source/Rtc/WebRTCConfig.h
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

add_library(clip255 STATIC ${CMAKE_SOURCE_DIR}/Source/ThirdParty/clip255.c)
target_link_libraries(metartccore7
        ${OPENSSL_LIBRARIES}
        ${usrsctp_BINARY_DIR}/usrsctplib/libusrsctp.dylib
        libSRTP::srtp2
        speexdsp
        opus
        clip255
        PkgConfig::FFMPEG
        "-framework Foundation"
        "-framework AVFoundation"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        ${OPENH264_LIBRARY}
)
target_link_libraries(metartc7
        metartccore7
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        opus
        ixwebsocket
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
