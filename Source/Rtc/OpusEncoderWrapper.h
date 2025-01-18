#pragma once
#include <opus.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>

#include <cstdint>
#include <vector>
#include <iostream>

// Constants
const int RTP_VERSION = 2;           // Version RTP
const int PAYLOAD_TYPE = 111;        // Opus Payload Type

// Structure de l'en-tête RTP
struct RTPHeader {
    uint8_t v_p_x_cc;    // Version (2 bits), Padding (1 bit), Extension (1 bit), CSRC Count (4 bits)
    uint8_t m_pt;        // Marker (1 bit), Payload Type (7 bits)
    uint16_t sequenceNumber; // Sequence Number
    uint32_t timestamp;      // Timestamp
    uint32_t ssrc;           // Synchronization Source Identifier
};

inline void debugRTPHeader(const RTPHeader& header) {
    std::cout << "RTP Header Debug:" << std::endl;
    std::cout << "  Version: " << ((header.v_p_x_cc >> 6) & 0x03) << std::endl;
    std::cout << "  Padding: " << ((header.v_p_x_cc >> 5) & 0x01) << std::endl;
    std::cout << "  Extension: " << ((header.v_p_x_cc >> 4) & 0x01) << std::endl;
    std::cout << "  CSRC Count: " << (header.v_p_x_cc & 0x0F) << std::endl;
    std::cout << "  Marker: " << ((header.m_pt >> 7) & 0x01) << std::endl;
    std::cout << "  Payload Type: " << (header.m_pt & 0x7F) << std::endl;
    std::cout << "  Sequence Number: " << ntohs(header.sequenceNumber) << std::endl;
    std::cout << "  Timestamp: " << ntohl(header.timestamp) << std::endl;
    std::cout << "  SSRC: " << ntohl(header.ssrc) << std::endl;
}

inline std::vector<uint8_t> createRTPPacket(const std::vector<uint8_t>& audioData, uint16_t seqNum, uint32_t ts, uint32_t ssrc) {
    RTPHeader header;
    header.v_p_x_cc = (RTP_VERSION << 6) | (0 << 5) | (0 << 4) | (0); // Version=2, Padding=0, Extension=0, CC=0
    header.m_pt = (0 << 7) | PAYLOAD_TYPE; // Marker=0, PayloadType=111
    header.sequenceNumber = htons(seqNum);
    header.timestamp = htonl(ts);
    header.ssrc = htonl(ssrc);
    debugRTPHeader(header);

    // Construire le paquet RTP
    std::vector<uint8_t> packet(sizeof(RTPHeader) + audioData.size());
    memcpy(packet.data(), &header, sizeof(RTPHeader)); // Copier l'en-tête RTP
    memcpy(packet.data() + sizeof(RTPHeader), audioData.data(), audioData.size()); // Ajouter les données audio

    return packet;
}


class OpusEncoderWrapper
{
public:
    OpusEncoderWrapper(int sampleRate, int channels, int bitrate)
    {
        // Validation des paramètres
        if (sampleRate != 8000 && sampleRate != 12000 && sampleRate != 16000 &&
            sampleRate != 24000 && sampleRate != 48000)
            throw std::invalid_argument("Invalid sample rate for Opus encoder");
        if (channels != 1 && channels != 2)
            throw std::invalid_argument("Invalid channel count for Opus encoder");

        int error;
        encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
        if (error != OPUS_OK)
            throw std::runtime_error("Failed to create Opus encoder");

        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate > 0 ? bitrate : OPUS_AUTO));
        opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(10));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
        opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
    }

    ~OpusEncoderWrapper()
    {
        opus_encoder_destroy(encoder);
    }

    std::vector<uint8_t> encode(const float* pcmData, int frameSize)
    {
        // Taille tampon ajustée à la taille maximale attendue
        std::vector<uint8_t> encodedData(4000);

        // Encodage des données
        int bytes = opus_encode_float(encoder, pcmData, frameSize, encodedData.data(), encodedData.size());
        if (bytes < 0)
            throw std::runtime_error("Opus encoding error: " + std::to_string(bytes));

        encodedData.resize(bytes);
        std::vector<uint8_t> rtpPacket = createRTPPacket(encodedData, sequenceNumber++, timestamp, ssrc);// Exemple de séquence et timestamp
        return rtpPacket;
    }

private:
    OpusEncoder* encoder;
    uint16_t sequenceNumber = 0; // Vous devez incrémenter cela à chaque envoi
    uint32_t timestamp = 0;       // Définissez cela selon votre logique
    uint32_t ssrc = 12345;
};
