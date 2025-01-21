#pragma once
#include <opus.h>
#include <iostream>
#include "RTPWrapper.h"

class OpusEncoderWrapper {
public:
    OpusEncoderWrapper(int sampleRate, int channels, int bitDepth) {
        // Validation des paramètres
        if (sampleRate != 8000 && sampleRate != 12000 && sampleRate != 16000 &&
            sampleRate != 24000 && sampleRate != 48000)
            throw std::invalid_argument("Invalid sample rate for Opus encoder " + std::to_string(sampleRate));
        if (channels != 1 && channels != 2)
            throw std::invalid_argument("Invalid channel count for Opus encoder");

        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder");

        auto bitrate = sampleRate * channels * bitDepth;
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate > 0 ? bitrate : OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
    }

    ~OpusEncoderWrapper() {
        opus_encoder_destroy(encoder);
    }

    std::vector<uint8_t> encode(float *pcmData, int frameSize) {
        // Taille tampon ajustée à la taille maximale attendue
        std::vector<uint8_t> encodedData(4000);

        // Encodage des données
        int bytes = opus_encode_float(encoder, pcmData, frameSize, encodedData.data(), encodedData.size());
        if (bytes < 0)
            throw std::runtime_error("Opus encoding error: " + std::to_string(bytes));

        encodedData.resize(bytes);
        std::vector<uint8_t> rtpPacket = RTPWrapper::createRTPPacket(encodedData, sequenceNumber++, timestamp, ssrc);
        if (rtpPacket.size() > 64) {
            debugPacket(encodedData);
        }
        return rtpPacket;
    }

private:
    OpusEncoder *encoder;
    uint16_t sequenceNumber = 0; // Vous devez incrémenter cela à chaque envoi
    uint32_t timestamp = 0; // Définissez cela selon votre logique
    uint32_t ssrc = 12345;

    static void debugPacket(const std::vector<uint8_t> &encodedData) {
        typedef struct {
            unsigned char cc: 4; /* CSRC count             */
            unsigned char x: 1; /* header extension flag  */
            unsigned char p: 1; /* padding flag           */
            unsigned char version: 2; /* protocol version       */
            unsigned char pt: 7; /* payload type           */
            unsigned char m: 1; /* marker bit             */
            uint16_t seq; /* sequence number        */
            uint32_t ts; /* timestamp              */
            uint32_t ssrc; /* synchronization source */
        } srtp_hdr_t;

        try {
            auto data = reinterpret_cast<const std::byte *>(encodedData.data());
            auto bin = rtc::binary(data, data + encodedData.size()); // message_variant
            auto message = rtc::make_message(std::move(bin));

            int size = int(message->size());
            auto message2 = rtc::make_message(size + 16 + 128, message);
            void *message2Data = message2->data();
            auto hdr = (srtp_hdr_t *) message2Data;
            std::cout << "RTP Header:" << std::endl;
            std::cout << "  Version: " << static_cast<int>(hdr->version)
                    << " (0x" << std::hex << static_cast<int>(hdr->version) << std::dec << ")" << std::endl;

            std::cout << "  CC: " << static_cast<int>(hdr->cc)
                    << " (0x" << std::hex << static_cast<int>(hdr->cc) << std::dec << ")" << std::endl;

            std::cout << "  X: " << static_cast<int>(hdr->x)
                    << " (0x" << std::hex << static_cast<int>(hdr->x) << std::dec << ")" << std::endl;

            std::cout << "  Padding: " << static_cast<int>(hdr->p)
                    << " (0x" << std::hex << static_cast<int>(hdr->p) << std::dec << ")" << std::endl;

            std::cout << "  Payload Type: " << static_cast<int>(hdr->pt)
                    << " (0x" << std::hex << static_cast<int>(hdr->pt) << std::dec << ")" << std::endl;

            std::cout << "  Marker (M): " << static_cast<int>(hdr->m)
                    << " (0x" << std::hex << static_cast<int>(hdr->m) << std::dec << ")" << std::endl;

            std::cout << "  Sequence Number: " << hdr->seq
                    << " (0x" << std::hex << hdr->seq << std::dec << ")" << std::endl;

            std::cout << "  Timestamp: " << hdr->ts
                    << " (0x" << std::hex << hdr->ts << std::dec << ")" << std::endl;

            std::cout << "  SSRC: " << hdr->ssrc
                    << " (0x" << std::hex << hdr->ssrc << std::dec << ")" << std::endl;
        } catch (std::exception &e) {
            std::cout << ("Error decoding RTP header: ") << e.what() << std::endl;
        }
    }
};

// const int frameSize = 960; // Taille de trame pour 20ms @ 48kHz (960 échantillons par canal)
// const int maxPacketSize = 520; // Taille maximale du paquet RTP (en octets)
//
// Diviser les données PCM en trames de taille appropriée
// for (size_t offset = 0; offset < pcmData.size(); offset += frameSize * 2) {
//     auto frameEnd = std::min(offset + frameSize * 2, pcmData.size());
//     std::vector<int16_t> frame(pcmData.begin() + offset, pcmData.begin() + frameEnd);
//     if (frame.size() < frameSize * 2) {
//         frame.resize(frameSize * 2, 0);
//     }
//
//     std::vector<float> floatFrame(frameSize * 2);
//     for (size_t i = 0; i < frame.size(); ++i) {
//         floatFrame[i] = static_cast<float>(frame[i]) / 32768.0f;
//     }
//
//     std::vector<uint8_t> encodedData;
//     try {
//         encodedData = opusEncoder.encode(floatFrame.data(), frameSize);
//         if (encodedData.size() > maxPacketSize) {
//             juce::Logger::outputDebugString("Encoded packet exceeds maximum size!");
//             continue;
//         }
//         if (encodedData.size() < 64) {
//             continue;
//         }
//     } catch (const std::exception &e) {
//         juce::Logger::outputDebugString("Error encoding audio frame: " + std::string(e.what()));
//         continue;
//     }
//
//
// }