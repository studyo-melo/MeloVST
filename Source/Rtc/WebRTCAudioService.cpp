#include "WebRTCAudioService.h"

WebRTCAudioService::WebRTCAudioService()
    : opusEncoder(48000, 2, 64000) { // Configuration : 48 kHz, stéréo, 64 kbps
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::pushAudioBuffer(const float* data, int numSamples) {
    // Convertir les échantillons float en PCM int16_t
    std::vector<int16_t> pcmData(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        pcmData[i] = static_cast<int16_t>(std::clamp(data[i], -1.0f, 1.0f) * 32767.0f);
    }

    // Ajouter les données PCM à la file d'attente thread-safe
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        audioQueue.push(std::move(pcmData));
    }
    queueCondition.notify_one();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    if (!audioTrack || !audioTrack->isOpen()) {
        return;
    }
    pushAudioBuffer(event.data, event.numSamples);
}

void WebRTCAudioService::startAudioThread() {
    juce::Logger::outputDebugString("Starting audio thread");
    stopAudioThread();
    stopThread = false;
    audioThread = std::thread([this]() {
        sendAudioData();
    });
}

void WebRTCAudioService::sendAudioData() {
    const int frameSize = 960; // Taille de trame pour 20ms @ 48kHz (960 échantillons par canal)
    const int maxPacketSize = 520; // Taille maximale du paquet RTP (en octets)

    while (!stopThread) {
        std::vector<int16_t> pcmData;

        // Récupérer les données PCM de la file d'attente
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() { return !audioQueue.empty() || stopThread; });
            if (stopThread) break;

            pcmData = std::move(audioQueue.front());
            audioQueue.pop();
        }

        // Diviser les données PCM en trames de taille appropriée
        for (size_t offset = 0; offset < pcmData.size(); offset += frameSize * 2) {
            auto frameEnd = std::min(offset + frameSize * 2, pcmData.size());
            std::vector<int16_t> frame(pcmData.begin() + offset, pcmData.begin() + frameEnd);
            if (frame.size() < frameSize * 2) {
                frame.resize(frameSize * 2, 0);
            }

            std::vector<float> floatFrame(frameSize * 2);
            for (size_t i = 0; i < frame.size(); ++i) {
                floatFrame[i] = static_cast<float>(frame[i]) / 32768.0f;
            }

            std::vector<uint8_t> encodedData;
            try {
                encodedData = opusEncoder.encode(floatFrame.data(), frameSize);
                if (encodedData.size() > maxPacketSize) {
                    juce::Logger::outputDebugString("Encoded packet exceeds maximum size!");
                    continue;
                }
                if (encodedData.size() < 64) {
                    continue;
                }
            } catch (const std::exception &e) {
                juce::Logger::outputDebugString("Error encoding audio frame: " + std::string(e.what()));
                continue;
            }

            // Envoyer le paquet encodé via le canal WebRTC
            if (audioTrack) {
                try {
                    juce::Logger::outputDebugString("Sending audio data: " + std::to_string(encodedData.size()) + " bytes");
                    debugPacket(encodedData);
                    rtc::InitLogger(rtc::LogLevel::Info);
                    rtc::InitLogger(rtc::LogLevel::Warning);
                    rtc::InitLogger(rtc::LogLevel::Debug);
                    audioTrack->send(reinterpret_cast<const std::byte *>(encodedData.data()), encodedData.size());
                } catch (const std::exception &e) {
                    juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
                }
            }
        }
    }
}

void WebRTCAudioService::debugPacket(const std::vector<uint8_t> &encodedData) {
    typedef struct {
        unsigned char cc : 4;      /* CSRC count             */
        unsigned char x : 1;       /* header extension flag  */
        unsigned char p : 1;       /* padding flag           */
        unsigned char version : 2; /* protocol version       */
        unsigned char pt : 7;      /* payload type           */
        unsigned char m : 1;       /* marker bit             */
        uint16_t seq;              /* sequence number        */
        uint32_t ts;               /* timestamp              */
        uint32_t ssrc;             /* synchronization source */
    } srtp_hdr_t;

    try {
        auto data = reinterpret_cast<const std::byte *>(encodedData.data());
        auto bin = rtc::binary(data,data + encodedData.size()); // message_variant
        auto message = rtc::make_message(std::move(bin));

        int size = int(message->size());
        auto message2 = rtc::make_message(size + 16+128, message);
        void* message2Data = message2->data();
        auto hdr = (srtp_hdr_t *)message2Data;
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
    }
    catch (std::exception &e) {
        std::cout << ("Error decoding RTP header: ") << e.what() << std::endl;
    }
}


void WebRTCAudioService::stopAudioThread() {
    juce::Logger::outputDebugString("Stopping audio thread");
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopThread = true;
    }
    queueCondition.notify_all();
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected) {
        startAudioThread();
    }
}
