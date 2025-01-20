#pragma once

#include <iostream>
#include <rtc/rtc.hpp>
#include "../Utils/VectorUtils.h"
#include <juce_core/juce_core.h>

#include "OpusEncoderWrapper.h"
#include "OpusDecoderWrapper.h"
#include "../Socket/WebSocketService.h"
#include "../Events/EventListener.h"
#include "../Models/Session.h"
#include "../Socket/SocketEvents.h"
#include "../Events/EventManager.h"
#include "../Utils/json.hpp"
#include "../Utils/FileUtils.h"
#include "ReconnectTimer.h"
#include "WebRTCConnexionHandler.h"

class WebRTCAudioService : public WebRTCConnexionHandler {
public:
    WebRTCAudioService();
    ~WebRTCAudioService();

private:
    OpusEncoderWrapper opusEncoder;
    OpusDecoderWrapper opusDecoder;
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

    static void debugPacket(const std::vector<uint8_t> &encodedData);
};
