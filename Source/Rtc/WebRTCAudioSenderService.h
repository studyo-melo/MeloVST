#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

#include "../Common/OpusCodecWrapper.h"
#include "../Api/WebSocketService.h"
#include "../Common/EventListener.h"
#include "../Models/Session.h"
#include "../Api/SocketEvents.h"
#include "../Common/EventManager.h"
#include "../ThirdParty/json.hpp"
#include "../Common/ResamplerWrapper.h"
#include "../Common/ReconnectTimer.h"
#include "WebRTCConnexionHandler.h"
#include "../Common/CircularBuffer.h"
#include "../Debug/DebugRTPWrapper.h"
#include "../AudioSettings.h"

class WebRTCAudioSenderService : public WebRTCConnexionHandler {
public:
    WebRTCAudioSenderService();

    ~WebRTCAudioSenderService();

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
    std::atomic<bool> threadRunning{true};
    std::thread encodingThread;

    // Tampon circulaire pour stocker les données audio (interleaved)
    CircularBuffer<float> circularBuffer; // implémentation à fournir ou basée sur juce::AbstractFifo
    juce::CriticalSection circularBufferLock;
};
