#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

#include "../Encoders/OpusCodecWrapper.h"
#include "../Socket/WebSocketService.h"
#include "../Events/EventListener.h"
#include "../Models/Session.h"
#include "../Socket/SocketEvents.h"
#include "../Events/EventManager.h"
#include "../Utils/json.hpp"
#include "../Utils/FileUtils.h"
#include "../Utils/ResamplerWrapper.h"
#include "ReconnectTimer.h"
#include "WebRTCConnexionHandler.h"
#include "../Debug/CircularBuffer.h"
#include "../Files/OpusFileHandler.h"
#include "../Files/WavFileHandler.h"

#define SAMPLE_RATE 48000
#define BITRATE 64000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16
#define OPUS_FRAME_SIZE 20
#define OPUS_SAMPLE_RATE 44100

class WebRTCAudioService : public WebRTCConnexionHandler {
public:
    WebRTCAudioService();
    ~WebRTCAudioService();
    void createFiles();
    void finalizeFiles();
private:
    void stopAudioThread();
    void startAudioThread();
    void onRTCStateChanged(const RTCStateChangeEvent &event);
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event);
    void processingThreadFunction();

    OpusCodecWrapper opusCodec;
    ResamplerWrapper resampler;
    uint16_t seqNum = 1; // Numéro de séquence RTP
    uint32_t timestamp = 0; // Timestamp RTP (incrementé à chaque trame)
    uint32_t ssrc = 12345; // Identifiant SSRC unique

    // Variables pour gérer le thread d'encodage
    std::atomic<bool> threadRunning { true };
    std::thread encodingThread;

    // Tampon circulaire pour stocker les données audio (interleaved)
    CircularBuffer<float> circularBuffer;  // implémentation à fournir ou basée sur juce::AbstractFifo
    juce::CriticalSection circularBufferLock;

    // Utilitaires de fichiers
    WavFileHandler vanillaWavFile;
    WavFileHandler decodedWavFileHandler;
    OpusFileHandler encodedOpusFileHandler;

    int currentNumSamples = 0;
    int currentSampleIndex = 0;

    double currentSampleRate = SAMPLE_RATE;
};
