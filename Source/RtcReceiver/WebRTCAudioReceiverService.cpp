#include "WebRTCAudioReceiverService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>
#include "../Api/SocketRoutes.h"
#include "../Utils/VectorUtils.h"

WebRTCAudioReceiverService::WebRTCAudioReceiverService(): WebRTCReceiverConnexionHandler(
                                                              WsRoute::GetOngoingSessionRTCVoice),
                                                          opusCodec(AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels(),
                                                                    AudioSettings::getInstance().getLatency(),
                                                                    AudioSettings::getInstance().getOpusBitRate()),
                                                          resampler(AudioSettings::getInstance().getSampleRate(),
                                                                    AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels()),
                                                          circularBuffer(
                                                              AudioSettings::getInstance().getSampleRate() *
                                                              AudioSettings::getInstance().getNumChannels()) {
}

WebRTCAudioReceiverService::~WebRTCAudioReceiverService() {}

void WebRTCAudioReceiverService::onAudioBlockReceived(const AudioBlockReceivedEvent &event) {
    const uint64_t timestamp = event.timestamp;
    std::vector<uint8_t> res;

    // Vérification et extraction sécurisée des données
    if (std::holds_alternative<std::string>(event.data)) {
        const std::string& audioString = std::get<std::string>(event.data);
        res.assign(audioString.begin(), audioString.end()); // Conversion string -> vector<uint8_t>
    }
    else if (std::holds_alternative<rtc::binary>(event.data)) {
        const rtc::binary& audioBinary = std::get<rtc::binary>(event.data);
        res.reserve(audioBinary.size()); // Évite les reallocations
        for (std::byte b : audioBinary) {
            res.push_back(static_cast<uint8_t>(b)); // Conversion std::byte -> uint8_t
        }
    }
    else {
        std::cerr << "[WebRTC] Erreur : format audio inconnu." << std::endl;
        return;
    }

    // Vérifier si les données sont valides
    if (res.empty()) {
        std::cerr << "[WebRTC] Paquet audio vide reçu." << std::endl;
        return;
    }

    // Décodage Opus en float PCM
    std::vector<float> decodedPacket = opusCodec.decode_float(res);
    if (decodedPacket.empty()) {
        std::cerr << "[WebRTC] Erreur de décodage Opus." << std::endl;
        return;
    }

    // Conversion des float 8-bit en float 32-bit (si nécessaire)
    std::vector<float> floatPacket = VectorUtils::convertFloatInt8ToFloat(decodedPacket.data(), decodedPacket.size());

    // Notification de l'événement avec les données décodées
    EventManager::getInstance().notifyOnAudioBlockReceivedDecoded(AudioBlockReceivedDecodedEvent{floatPacket, timestamp});
}


