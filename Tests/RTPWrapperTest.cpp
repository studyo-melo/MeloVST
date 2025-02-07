//
// Created by Padoa on 24/01/2025.
//
#include <juce_core/juce_core.h>
#include "Common/RTPWrapper.h"

class RTPWrapperTest : public juce::UnitTest
{
public:
    RTPWrapperTest() : juce::UnitTest("RTPWrapperTest") {}

    void runTest() override
    {
        // Test de la création d'un paquet RTP
        beginTest("Create RTP Packet");
        std::vector<uint8_t> audioData = {0x01, 0x02, 0x03, 0x04, 0x05}; // Données audio d'exemple
        uint16_t seqNum = 1;
        uint32_t timestamp = 1000;
        uint32_t ssrc = 123456;

        std::vector<uint8_t> packet = RTPWrapper::createRTPPacket(audioData, seqNum, timestamp, ssrc);
        expect(packet.size() == sizeof(RTPHeader) + audioData.size(), "Packet size mismatch");

        RTPHeader* header = reinterpret_cast<RTPHeader*>(packet.data());
        expect(((header->v_p_x_cc >> 6) & 0x03) == RTP_VERSION, "Version mismatch");
        expect(header->sequenceNumber == htons(seqNum), "Sequence number mismatch");
        expect(header->timestamp == htonl(timestamp), "Timestamp mismatch");
        expect(header->ssrc == htonl(ssrc), "SSRC mismatch");

        // Vérification des données audio
        std::vector<uint8_t> extractedAudio(packet.begin() + sizeof(RTPHeader), packet.end());
        expect(extractedAudio == audioData, "Audio data mismatch");

        // Test de la suppression de l'en-tête RTP
        beginTest("Remove RTP Header");
        std::vector<uint8_t> extractedAudioData = RTPWrapper::removeRTPHeader(packet);
        expect(extractedAudioData == audioData, "Failed to remove RTP header");

        // Test de la suppression avec un paquet trop petit
        beginTest("Remove RTP Header Too Small Packet");
        std::vector<uint8_t> smallPacket = {0x00}; // Paquet trop petit
        expectExceptionThrown([&]() { RTPWrapper::removeRTPHeader(smallPacket); }, "Invalid RTP packet: too small to contain a header");

        // Test de la suppression avec une version non prise en charge
        beginTest("Remove RTP Header Unsupported Version");
        // sizeof(RTPHeader) = 12
        std::vector<uint8_t> invalidPacket = std::vector<u_int8_t>(); // Version invalide
        invalidPacket.resize(16);
        invalidPacket.push_back(0xFF); // Version 0
        expectExceptionThrown([&]() { RTPWrapper::removeRTPHeader(invalidPacket); }, "Invalid RTP packet: unsupported version");

        // Test de la suppression sans données utiles
        beginTest("Remove RTP Header No Payload Data");
        std::vector<uint8_t> noPayloadPacket = {0x80, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00}; // En-tête valide mais pas de données
        noPayloadPacket.resize(12);
        expectExceptionThrown([&]() { RTPWrapper::removeRTPHeader(noPayloadPacket); }, "Invalid RTP packet: no payload data");
    }

private:
    void expectExceptionThrown(std::function<void()> func, const std::string& expectedMessage)
    {
        try
        {
            func(); // Essayer d'exécuter la fonction donnée
            expect(false, "Expected exception was not thrown");
        }
        catch (const std::runtime_error& e)
        {
            // Vérifiez si le message d'exception correspond
            expect(e.what() == expectedMessage, "Unexpected exception message");
        }
        catch (...)
        {
            expect(false, "Expected exception was not of type std::runtime_error");
        }
    }
};

static RTPWrapperTest rtpWrapperTest; // Créez une instance pour enregistrer le test
