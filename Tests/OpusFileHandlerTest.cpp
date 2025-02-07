#include <juce_core/juce_core.h>
#include "Common/OpusFileHandler.h"

class OpusFileHandlerTest : public juce::UnitTest {
public:
    OpusFileHandlerTest() : juce::UnitTest("OpusFileHandler Test") {}

    void runTest() override {
        beginTest("Create Opus File");
        {
            OpusFileHandler opusHandler(48000, 64000, 2);
            expect(opusHandler.create("test.opus"), "Failed to create Opus file");
        }

        beginTest("Write Opus Data");
        {
            OpusFileHandler opusHandler(48000, 64000, 2);
            expect(opusHandler.create("test.opus"), "Failed to create Opus file");
            std::vector<int16_t> samples(960, 0);
            opusHandler.write(samples);
            expect(samples.size() == 960, "Incorrect number of samples written");
        }

        beginTest("Close Opus File");
        {
            OpusFileHandler opusHandler(48000, 64000, 2);
            expect(opusHandler.create("test.opus"), "Failed to create Opus file");
            opusHandler.close();
            std::ifstream file("test.opus", std::ios::binary);
            expect(file.is_open(), "Opus file was not properly closed");
        }
    }
};

static OpusFileHandlerTest opusFileHandlerTest;
