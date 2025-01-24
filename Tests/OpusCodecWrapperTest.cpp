#include "Rtc/OpusCodecWrapper.h"
#include <juce_core/juce_core.h>
#include <vector>

class OpusCodecWrapperTest : public juce::UnitTest {
public:
    OpusCodecWrapperTest() : juce::UnitTest("OpusCodecWrapperTest") {}

    void runTest() override {
        testEncodingDecoding();
        // testInvalidInputs();
    }

private:
    void testEncodingDecoding() {
        beginTest("Encoding and Decoding");

        OpusCodecWrapper codec;

        // Simuler un tampon audio PCM (2 canaux, 48000 Hz, 20 ms de données)
        const int sampleRate = 48000;
        const int channels = 2;
        const int frameSize = static_cast<int>(0.02 * sampleRate); // 20 ms de données
        std::vector<float> pcmData(frameSize * channels, 0.5f); // Tampon rempli de données arbitraires

        // Encoder les données PCM
        auto encodedData = codec.encode(pcmData.data());
        expect(!encodedData.empty(), "Encoded data should not be empty");

        // Décoder les données encodées
        auto decodedData = codec.decode(encodedData);
        expect(!decodedData.empty(), "Decoded data should not be empty");
        expectEquals(static_cast<int>(decodedData.size()), frameSize * channels, "Decoded data size mismatch");
    }

    // void testInvalidInputs() {
    //     beginTest("Invalid Inputs");
    //
    //     OpusCodecWrapper codec;
    //
    //     // Cas : Décoder des données invalides
    //     std::vector<uint8_t> invalidData = {0x00, 0x01, 0x02}; // Données non valides
    //     bool decodingErrorCaught = false;
    //     try {
    //         codec.decode(invalidData);
    //     } catch (const std::runtime_error& e) {
    //         decodingErrorCaught = true;
    //     }
    //     expect(decodingErrorCaught, "Decoding invalid data should throw an error");
    //
    //     // Cas : Encoder avec un pointeur nul
    //     bool encodingErrorCaught = false;
    //     try {
    //         codec.encode(nullptr);
    //     } catch (const std::runtime_error& e) {
    //         encodingErrorCaught = true;
    //     }
    //     expect(encodingErrorCaught, "Encoding with a null pointer should throw an error");
    // }
};

// Inscription du test
static OpusCodecWrapperTest opusCodecWrapperTest;
