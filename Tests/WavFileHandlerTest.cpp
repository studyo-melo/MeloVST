#include <juce_core/juce_core.h>
#include "Common/WavFileHandler.h"
class WavFileHandlerTest : public juce::UnitTest {
public:
    WavFileHandlerTest() : juce::UnitTest("WavFileHandler Test") {}

    void runTest() override {
        beginTest("Create WAV File");
        {
            WavFileHandler wavHandler;
            expect(wavHandler.create("test.wav", 44100, 16, 2), "Failed to create WAV file");
        }

        beginTest("Write WAV Data");
        {
            WavFileHandler wavHandler;
            expect(wavHandler.create("test.wav", 44100, 16, 2), "Failed to create WAV file");
            std::vector<int16_t> samples(44100, 0);
            wavHandler.write(samples);
            expect(samples.size() == 44100, "Incorrect number of samples written");
        }

        beginTest("Close WAV File");
        {
            WavFileHandler wavHandler;
            expect(wavHandler.create("test.wav", 44100, 16, 2), "Failed to create WAV file");
            wavHandler.close();
            std::ifstream file("test.wav", std::ios::binary);
            expect(file.is_open(), "WAV file was not properly closed");
        }
    }
};

static WavFileHandlerTest wavFileHandlerTest;
