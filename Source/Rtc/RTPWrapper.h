#pragma once
#include <iostream>

// Constants
const int RTP_VERSION = 2; // Version RTP
// const int PAYLOAD_TYPE = 111;  // Opus Payload Type
const int PAYLOAD_TYPE = 127; // Opus Payload Type

struct RTPHeader {
    uint8_t v_p_x_cc; // Version (2 bits), Padding (1 bit), Extension (1 bit), CSRC Count (4 bits)
    uint8_t m_pt; // Marker (1 bit), Payload Type (7 bits)
    uint16_t sequenceNumber; // Sequence Number
    uint32_t timestamp; // Timestamp
    uint32_t ssrc; // Synchronization Source Identifier
};

class RTPWrapper {
public:
  static void debugRTPHeader(const RTPHeader &header) {
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

    static std::vector<uint8_t> createRTPPacket(const std::vector<uint8_t> &audioData, uint16_t seqNum, uint32_t ts,
                                                uint32_t ssrc) {
        RTPHeader header;
        header.v_p_x_cc = (RTP_VERSION << 6) | (0 << 5) | (0 << 4) | (0); // Version=2, Padding=0, Extension=0, CC=0
        header.m_pt = (0 << 7) | PAYLOAD_TYPE; // Marker=0, PayloadType=111
        header.sequenceNumber = htons(seqNum);
        header.timestamp = htonl(ts);
        header.ssrc = htonl(ssrc);
        // debugRTPHeader(header);

        // Construire le paquet RTP
        std::vector<uint8_t> packet(sizeof(RTPHeader) + audioData.size());
        memcpy(packet.data(), &header, sizeof(RTPHeader)); // Copier l'en-tête RTP
        memcpy(packet.data() + sizeof(RTPHeader), audioData.data(), audioData.size()); // Ajouter les données audio

        return packet;
    }
};