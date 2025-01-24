#pragma once
#include <iostream>

// Constants
const int RTP_VERSION = 2; // Version RTP
const int PAYLOAD_TYPE = 111;  // Opus Payload Type
// const int PAYLOAD_TYPE = 127; // Opus Payload Type

struct RTPHeader {
    uint8_t v_p_x_cc; // Version (2 bits), Padding (1 bit), Extension (1 bit), CSRC Count (4 bits)
    uint8_t m_pt; // Marker (1 bit), Payload Type (7 bits)
    uint16_t sequenceNumber; // Sequence Number
    uint32_t timestamp; // Timestamp
    uint32_t ssrc; // Synchronization Source Identifier
};

class RTPWrapper {
public:
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

    static std::vector<uint8_t> removeRTPHeader(const std::vector<uint8_t>& rtpPacket) {
      // Vérifier que le paquet est assez grand pour contenir un en-tête RTP
      if (rtpPacket.size() < 12) {
          throw std::runtime_error("Invalid RTP packet: too small to contain a header");
      }

      // Décoder les champs de l'en-tête RTP
      const uint8_t v_p_x_cc = rtpPacket[0];
      const uint8_t version = (v_p_x_cc >> 6) & 0x03;

      // Vérifier la version RTP (doit être 2)
      if (version != 2) {
          throw std::runtime_error("Invalid RTP packet: unsupported version");
      }

      // Compter les CSRC
      const uint8_t csrcCount = v_p_x_cc & 0x0F;
      size_t headerSize = 12 + (csrcCount * 4); // Taille minimale = 12 octets, plus 4 octets par CSRC

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


    // DEBUG

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
};

