#include "WebRTCAudioService.h"

#include "../Debug/DebugRTPWrapper.h"
#include "../Utils/AudioSettings.h"
#include "../Utils/AudioUtils.h"

#define SAMPLE_RATE 48000
#define BITRATE 64000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16
#define OPUS_FRAME_SIZE 20
#define OPUS_SAMPLE_RATE 48000

WebRTCAudioService::WebRTCAudioService(): circularBuffer(SAMPLE_RATE * NUM_CHANNELS),
                                          vanillaWavFile(SAMPLE_RATE, NUM_CHANNELS),
                                          decodedWavFileHandler(SAMPLE_RATE, NUM_CHANNELS),
                                          encodedOpusFileHandler(SAMPLE_RATE, BITRATE, NUM_CHANNELS),
                                          opusCodec(OPUS_SAMPLE_RATE, NUM_CHANNELS, OPUS_FRAME_SIZE),
                                          resampler(SAMPLE_RATE, OPUS_SAMPLE_RATE, NUM_CHANNELS) {
}

WebRTCAudioService::~WebRTCAudioService() {
    stopAudioThread();
}

void WebRTCAudioService::onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) {
    std::vector<float> audioBlock = event.data;
    currentNumSamples = event.numSamples;
    currentSampleIndex = 0;

    // On vérifie que le buffer audio (audioBlock) contient des données
    if (!audioBlock.empty()) {
        const int numSamples = static_cast<int>(audioBlock.size()); {
            juce::ScopedLock sl(circularBufferLock);
            circularBuffer.pushSamples(audioBlock.data(), numSamples);
        }
        // On vide le buffer temporaire pour éviter une réécriture multiple
        audioBlock.clear();
    }
}

void WebRTCAudioService::processingThreadFunction() {
    // Calcul du nombre d'échantillons par canal pour une trame de 20 ms
    const int frameSamples = static_cast<int>(SAMPLE_RATE * OPUS_FRAME_SIZE / 1000.0); // 48000 * 20/1000 = 960
    const int totalFrameSamples = frameSamples * NUM_CHANNELS; // Pour un signal interleaved
    while (threadRunning) {
        bool frameAvailable = false;
        std::vector<float> frameData(totalFrameSamples); {
            juce::ScopedLock sl(circularBufferLock);
            if (circularBuffer.getNumAvailableSamples() >= totalFrameSamples) {
                circularBuffer.popSamples(frameData.data(), totalFrameSamples);
                frameAvailable = true;
            }
        }

        if (frameAvailable) {
            vanillaWavFile.write(frameData, false);
            EventManager::getInstance().notifyOnAudioBlockSent(AudioBlockSentEvent{frameData});

            std::vector<unsigned char> opusPacket = opusCodec.encode_float(frameData, frameSamples);
            if (opusPacket.empty()) {
                return;
            }
            encodedOpusFileHandler.write(opusPacket);
            try {
                std::vector<float> decodedFrame = opusCodec.decode_float(opusPacket);
                if (!decodedFrame.empty()) {
                    decodedWavFileHandler.write(decodedFrame, false);
                }
            } catch (std::exception &e) {
                juce::Logger::outputDebugString("Error decoding opus packet: " + std::string(e.what()));
            }
            if (audioTrack) {
                try {
                    auto rtpPacket = RTPWrapper::createRTPPacket(opusPacket, seqNum++, timestamp, ssrc);
                    timestamp += frameSamples;
                    juce::Logger::outputDebugString(
                        "Sending audio data: " + std::to_string(rtpPacket.size()) + " bytes");
                    DebugRTPWrapper::debugPacket(rtpPacket);

                    audioTrack->send(reinterpret_cast<const std::byte *>(rtpPacket.data()), rtpPacket.size());
                } catch (const std::exception &e) {
                    juce::Logger::outputDebugString("Error sending audio data: " + std::string(e.what()));
                }
            }
        } else {
            // Si pas assez d'échantillons, on attend un peu
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


void WebRTCAudioService::startAudioThread() {
    encodingThread = std::thread(&WebRTCAudioService::processingThreadFunction, this);
    threadRunning = true;
}

void WebRTCAudioService::stopAudioThread() {
    threadRunning = false;
    if (encodingThread.joinable())
        encodingThread.join();
}

void WebRTCAudioService::onRTCStateChanged(const RTCStateChangeEvent &event) {
    if (event.state == rtc::PeerConnection::State::Connected && !threadRunning) {
        createFiles();
        startAudioThread();
    } else {
        stopAudioThread();
    }
}


void WebRTCAudioService::createFiles() {
    vanillaWavFile.create("1_base_audio.wav");
    encodedOpusFileHandler.create("2_encoded_opus_audio.opus");
    decodedWavFileHandler.create("1_decoded_opus_audio.wav");
}

void WebRTCAudioService::finalizeFiles() {
    vanillaWavFile.close();
    encodedOpusFileHandler.close();
    decodedWavFileHandler.close();
}
