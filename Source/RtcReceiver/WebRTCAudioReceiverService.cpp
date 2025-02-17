#include "WebRTCAudioReceiverService.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"
#include "../Common/EventManager.h"
#include <rtc/rtc.hpp>
#include "../Api/SocketRoutes.h"
#include "../Utils/VectorUtils.h"
int error = 0;

WebRTCAudioReceiverService::WebRTCAudioReceiverService(): WebRTCReceiverConnexionHandler(
                                                              WsRoute::GetOngoingSessionRTCVoice),
                                                          opusCodec(AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels(),
                                                                    AudioSettings::getInstance().getLatency(),
                                                                    AudioSettings::getInstance().getOpusBitRate()),
                                                          resampler(AudioSettings::getInstance().getSampleRate(),
                                                                    AudioSettings::getInstance().getOpusSampleRate(),
                                                                    AudioSettings::getInstance().getNumChannels()),
                                                          decoder(opus_decoder_create(48000, 1, &error))
{
    if (error != OPUS_OK) {
        std::cerr << "Échec de l'initialisation du décodeur Opus: " << opus_strerror(error) << std::endl;
    }
}

WebRTCAudioReceiverService::~WebRTCAudioReceiverService() {
}

void WebRTCAudioReceiverService::onAudioBlockReceived(const AudioBlockReceivedEvent &event){
    // Récupération du message variant et du timestamp (fourni par l'événement)
    rtc::message_variant audioBlock = event.data;
    uint64_t timestamp = event.timestamp;

    // Si le message est un string, on l'ignore
    if (!std::holds_alternative<rtc::binary>(audioBlock))
        return;

    const rtc::binary& msg = std::get<rtc::binary>(audioBlock);
    size_t rtpHeaderSize = 28;
    rtc::binary encodedData(msg.begin() + rtpHeaderSize, msg.end());
    std::vector<unsigned char> encoded(msg.size());
    std::transform(encodedData.begin(), encodedData.end(), encoded.begin(), [](std::byte b) {
        return static_cast<unsigned char>(b);
    });

    auto packetFrameSize = opus_packet_get_samples_per_frame(encoded.data(), 48000);
    auto nbChannels = opus_packet_get_nb_channels(encoded.data());
    auto bandwidth = opus_packet_get_bandwidth(encoded.data());
    auto nbSamples = opus_packet_get_nb_samples(encoded.data(), encoded.size(), 48000);
    juce::Logger::outputDebugString("Audio block received");
    juce::Logger::outputDebugString("Size: " + std::to_string(encoded.size()));
    juce::Logger::outputDebugString("Frame size: " + std::to_string(packetFrameSize));
    juce::Logger::outputDebugString("Nb channels: " + std::to_string(nbChannels));
    juce::Logger::outputDebugString("Bandwidth: " + std::to_string(bandwidth));
    juce::Logger::outputDebugString("Nb samples: " + std::to_string(nbSamples));

    // Extraction du timestamp contenu dans le message (les 8 premiers octets)
    // std::memcpy(&timestamp, msg.data(), sizeof(uint64_t));

    // Extraction des données audio Opus (après le timestamp)
    // rtc::binary encodedData(msg.begin(), msg.end());

    // Décodage immédiat sans tampon (pas de mutex nécessaire)
    constexpr int maxFrameSize = 960; // par exemple, 20ms à 48 kHz pour 1 canal
    std::vector<float> pcmBuffer(maxFrameSize, 0.0f);

    int frameSize = opus_decode_float(decoder,
                                      encoded.data(),
                                      encoded.size(),
                                      pcmBuffer.data(),
                                      maxFrameSize,
                                      0);
    if (frameSize < 0)
    {
        std::cerr << "Erreur lors du décodage Opus : " << opus_strerror(frameSize) << std::endl;
        return;
    }

    // Ajustement de la taille du buffer pour contenir uniquement les échantillons décodés
    pcmBuffer.resize(frameSize);
    EventManager::getInstance().notifyOnAudioBlockReceivedDecoded(AudioBlockReceivedDecodedEvent{pcmBuffer, timestamp});
}
