#pragma once
#include <vector>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h> // Pour htons et htonl

// Constants
constexpr int RTP_VERSION = 2;
constexpr int PAYLOAD_TYPE = 111;
constexpr int PADDING = 0;
constexpr int EXTENSION = 0;
constexpr int CC = 0;
constexpr int MARKER = 0;

struct RTPHeader {
    uint8_t v_p_x_cc; // Version (2 bits), Padding (1 bit), Extension (1 bit), CSRC Count (4 bits)
    uint8_t m_pt;     // Marker (1 bit), Payload Type (7 bits)
    uint16_t sequenceNumber; // Sequence Number
    uint32_t timestamp; // Timestamp
    uint32_t ssrc;     // Synchronization Source Identifier
};

class RTPWrapper {
public:
    static std::vector<uint8_t> createRTPPacket(const std::vector<unsigned char>& audioData, uint16_t seqNum, uint32_t ts, uint32_t ssrc) {
        RTPHeader header;
        header.v_p_x_cc = (RTP_VERSION << 6) | (PADDING << 5) | (EXTENSION << 4) | CC; // Version=2, Padding=0, Extension=0, CC=0
        header.m_pt = (MARKER << 7) | PAYLOAD_TYPE; // Marker=0, PayloadType=111
        header.sequenceNumber = htons(seqNum);
        header.timestamp = htonl(ts);
        header.ssrc = htonl(ssrc);

        // Construire le paquet RTP
        std::vector<uint8_t> packet(sizeof(RTPHeader) + audioData.size());
        memcpy(packet.data(), &header, sizeof(RTPHeader)); // Copier l'en-tête RTP
        memcpy(packet.data() + sizeof(RTPHeader), audioData.data(), audioData.size()); // Ajouter les données audio

        return packet;
    }

    static std::vector<uint8_t> removeRTPHeader(const std::vector<uint8_t>& rtpPacket) {
        // Vérifier que le paquet est assez grand pour contenir un en-tête RTP
        if (rtpPacket.size() < sizeof(RTPHeader)) {
            throw std::runtime_error("Invalid RTP packet: too small to contain a header");
        }

        // Décoder les champs de l'en-tête RTP
        const uint8_t v_p_x_cc = rtpPacket[0];
        const uint8_t version = (v_p_x_cc >> 6) & 0x03;

        // Vérifier la version RTP (doit être 2)
        if (version != RTP_VERSION) {
            throw std::runtime_error("Invalid RTP packet: unsupported version");
        }

        // Compter les CSRC
        const uint8_t csrcCount = v_p_x_cc & 0x0F;
        size_t headerSize = sizeof(RTPHeader) + (csrcCount * 4); // Taille minimale = 12 octets, plus 4 octets par CSRC

        // Vérifier s'il y a une extension RTP
        const uint8_t extensionFlag = (v_p_x_cc >> 4) & 0x01;
        if (extensionFlag) {
            if (rtpPacket.size() < headerSize + 4) {
                throw std::runtime_error("Invalid RTP packet: not enough data for extension header");
            }
            // Lire la longueur de l'extension
            const uint16_t extensionLength = ntohs(*reinterpret_cast<const uint16_t*>(&rtpPacket[headerSize + 2]));
            headerSize += 4 + (extensionLength * 4);
        }

        // Vérifier la taille totale du paquet
        if (rtpPacket.size() <= headerSize) {
            throw std::runtime_error("Invalid RTP packet: no payload data");
        }

        // Retourner uniquement le payload (sans l'en-tête RTP)
        return std::vector<uint8_t>(rtpPacket.begin() + headerSize, rtpPacket.end());
    }
};
