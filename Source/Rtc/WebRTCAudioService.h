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

class WebRTCAudioService : public WebRTCConnexionHandler {
public:
    WebRTCAudioService();
    ~WebRTCAudioService();

private:
    OpusCodecWrapper opusCodec;
    ResamplerWrapper resampler;
    uint16_t seqNum = 1; // Numéro de séquence RTP
    uint32_t timestamp = 0; // Timestamp RTP (incrementé à chaque trame)
    uint32_t ssrc = 12345; // Identifiant SSRC unique
    // Audio Thread
    std::thread audioThread;
    std::queue<std::vector<float>> audioQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool audioThreadRunning = false;
    bool stopThread = false;

    void stopAudioThread();
    void startAudioThread();
    void onRTCStateChanged(const RTCStateChangeEvent &event);
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event);
    void sendAudioData();
};
