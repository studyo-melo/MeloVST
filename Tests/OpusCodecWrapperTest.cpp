#include <CDSPResampler.h>

#include "Rtc/OpusCodecWrapper.h"
#include <juce_core/juce_core.h>
#include <vector>

#include "Utils/AudioUtils.h"
#include "Utils/VectorUtils.h"
#include <cmath>

#include "Utils/FileUtils.h"

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

        double frameDurationInMs = 0.02;
        const int sampleRate = 48000;
        const int channels = 2;
        const int frameSize = static_cast<int>(frameDurationInMs * sampleRate);
        OpusCodecWrapper codec(sampleRate, channels, 1000 * frameDurationInMs);

        std::vector<int16_t> inbuf = generate_music(frameSize * channels);

        auto encodedData = codec.encode(inbuf);
        expect(!encodedData.empty(), "Encoded data should not be empty");
        expect(encodedData.size() < inbuf.size(), "Encoded data size should be less than input data size");

        std::vector<int16_t> decodedData = codec.decode(encodedData);
        expect(!decodedData.empty(), "Decoded data should not be empty");
        expect(decodedData.size() > encodedData.size(), "Decoded data size should be greater than encoded data size");
        expectEquals(static_cast<int>(decodedData.size()), frameSize * channels, "Decoded data size mismatch");

        const int16_t tolerance = 10;
        for (int i = 0; i < frameSize * channels; ++i) {
            expectWithinAbsoluteError(decodedData[i],  inbuf[i], tolerance, "Decoded data mismatch");
        }
    }


    static std::vector<int16_t> generate_music(const int len) {
        // Paramètres pour le signal
        constexpr int sampleRate = 48000; // Fréquence d'échantillonnage
        constexpr double frequencyLeft = 440.0; // Fréquence pour le canal gauche (A4 - 440 Hz)
        constexpr double frequencyRight = 550.0; // Fréquence pour le canal droit (C#5 - 550 Hz)
        constexpr double amplitude = 30000.0; // Amplitude (valeur maximale pour int16_t = 32767)

        // Vecteur pour contenir les données générées
        std::vector<int16_t> buf(len, 0);

        // Génération d'un signal sinusoïdal pour chaque canal
        for (int i = 0; i < len / 2; ++i) {
            double time = static_cast<double>(i) / sampleRate; // Temps actuel en secondes
            buf[i * 2] = static_cast<int16_t>(amplitude * std::sin(2.0 * M_PI * frequencyLeft * time));  // Canal gauche
            buf[i * 2 + 1] = static_cast<int16_t>(amplitude * std::sin(2.0 * M_PI * frequencyRight * time)); // Canal droit
        }

        return buf;
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
