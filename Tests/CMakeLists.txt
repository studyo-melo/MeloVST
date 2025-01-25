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
    target_link_libraries(${test_name} PRIVATE juce::juce_core)
    target_include_directories(${test_name} PRIVATE ../Source)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Ajouter les tests individuellement
add_unit_test(OpusCodecWrapperTest OpusCodecWrapperTest.cpp)
target_link_libraries(OpusCodecWrapperTest PRIVATE
        juce::juce_audio_basics
        juce::juce_dsp
        ${OPUS_LIBRARY}
)
add_unit_test(RTPWrapperTest RTPWrapperTest.cpp)
target_link_libraries(RTPWrapperTest PRIVATE
        datachannel
        datachannel_static INTERFACE datachannel
)

add_executable(RunAllTests
        OpusCodecWrapperTest.cpp
        RTPWrapperTest.cpp
        TestMain.cpp
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
