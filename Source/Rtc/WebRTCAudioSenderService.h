#pragma once

#include <iostream>
#include <juce_core/juce_core.h>

#include "../Common/OpusCodecWrapper.h"
#include "../Api/WebSocketService.h"
#include "../Common/EventListener.h"

#include "../Common/ResamplerWrapper.h"
#include "WebRTCConnexionHandler.h"
#include "../Common/CircularBuffer.h"

class WebRTCAudioSenderService final : public WebRTCConnexionHandler {
public:
    WebRTCAudioSenderService();
    ~WebRTCAudioSenderService() override;

private:
    void stopAudioThread();
    void startAudioThread();
    void onRTCStateChanged(const RTCStateChangeEvent &event) override;
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;
    void processingThreadFunction();

    OpusCodecWrapper opusCodec;
    ResamplerWrapper resampler;
    uint16_t seqNum = 1;
    uint32_t timestamp = 0;
    uint32_t ssrc = 12345;

    std::atomic<bool> threadRunning{true};
    std::thread encodingThread;

    CircularBuffer<float> circularBuffer;
    juce::CriticalSection circularBufferLock;
};
