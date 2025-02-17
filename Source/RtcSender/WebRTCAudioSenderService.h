#pragma once

#include <iostream>
#include <juce_core/juce_core.h>

#include "../Common/OpusEncoderWrapper.h"
#include "../Api/WebSocketService.h"
#include "../Common/EventListener.h"

#include "../Common/ResamplerWrapper.h"
#include "WebRTCSenderConnexionHandler.h"
#include "../Common/CircularBuffer.h"

class WebRTCAudioSenderService final : public WebRTCSenderConnexionHandler {
public:
    WebRTCAudioSenderService();
    ~WebRTCAudioSenderService() override;

private:
    void stopAudioThread();
    void startAudioThread();
    void onRTCStateChanged(const RTCStateChangeEvent &event) override;
    void onAudioBlockProcessedEvent(const AudioBlockProcessedEvent &event) override;
    void processingThreadFunction();

    OpusEncoderWrapper opusEncoder;
    ResamplerWrapper resampler;
    uint16_t seqNum = 1;
    uint32_t timestamp = 0;
    uint32_t ssrc = 12345;

    std::atomic<bool> threadRunning{true};
    std::thread encodingThread;

    CircularBuffer<float> circularBuffer;
    juce::CriticalSection circularBufferLock;
};
