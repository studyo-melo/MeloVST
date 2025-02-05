cmake_minimum_required(VERSION 3.15)
# Nom du projet
project(MeloVSTTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Activer les tests
enable_testing()

find_library(OPUS_LIBRARY opus REQUIRED)

# Fonction pour simplifier l'ajout de tests
function(add_unit_test test_name source_file)
    add_executable(${test_name} ${source_file} TestMain.cpp)
    target_link_libraries(${test_name}
            PUBLIC
            opus
            datachannel
            datachannel_static INTERFACE datachannel
            ${OPUS_LIBRARY}
            PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_dsp
    )
    target_include_directories(${test_name} PRIVATE ../Source)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Ajouter les tests individuellement
add_unit_test(OpusCodecWrapperTest OpusCodecWrapperTest.cpp)
add_unit_test(RTPWrapperTest RTPWrapperTest.cpp)
add_unit_test(WavFileHandlerTest WavFileHandlerTest.cpp)

add_executable(RunAllTests
        OpusCodecWrapperTest.cpp
        RTPWrapperTest.cpp
        TestMain.cpp
        WavFileHandlerTest.cpp
)

target_link_libraries(RunAllTests
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_dsp
        ${OPUS_LIBRARY}
)
target_include_directories(RunAllTests PRIVATE ../Source)
add_test(NAME RunAllTests COMMAND RunAllTests)
