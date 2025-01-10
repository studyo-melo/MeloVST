#include "MeloWebRTCServerService.h"

MeloWebRTCServerService::MeloWebRTCServerService() {
    rtc::InitLogger(rtc::LogLevel::Info);

    // Créez un serveur WebRTC
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    // Créez un transport peer-to-peer
    peerConnection = std::make_shared<rtc::PeerConnection>(config);

    peerConnection->onLocalDescription([](rtc::Description sdp) {
        // Send the SDP to the remote peer
        // MY_SEND_DESCRIPTION_TO_REMOTE(std::string(sdp));
    });


    // Configurez les callbacks
    peerConnection->onTrack([this](std::shared_ptr<rtc::Track> track) {
        std::cout << "Track received: " << track << std::endl;

        // Gérer l'audio ici (par ex., streamer vers l'application React)
    });

    peerConnection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
        std::cout << "Data channel opened: " << channel->label() << std::endl;
        dataChannel = channel;

        // Gérer les messages reçus (par exemple, commandes ou contrôle)
        dataChannel->onMessage(
        [](const rtc::binary& data) {
            std::cout << "Received binary data: " << data.size() << " bytes" << std::endl;
        },
        [](const std::string& data) {
            std::cout << "Received text data: " << data << std::endl;
        });
    });
        // Créer un canal de données pour transmettre l'audio
    dataChannel = peerConnection->createDataChannel("secure-audio");
    // peerConnection->setRemoteDescription("toto")
    peerConnection->setLocalDescription(rtc::Description::Type::Offer);

}

void MeloWebRTCServerService::sendAudioData(const float* data, int size) const {
    if (!dataChannel || !dataChannel->isOpen()) {
         std::cerr << "Data channel is not open!" << std::endl;
         return;
    }
    juce::Logger::outputDebugString("Sending audio data");
    // Convertir les données audio en format binaire
    rtc::binary binaryData = VectorUtils::convertFloatToBinary(data, size);

    // Envoyer les données via le canal de données
    dataChannel->send(binaryData);
}

void MeloWebRTCServerService::handleAnswer(const std::string& sdp) const {
    // Réception de la réponse SDP depuis le client React
    peerConnection->setRemoteDescription(rtc::Description(sdp, "answer"));
}
