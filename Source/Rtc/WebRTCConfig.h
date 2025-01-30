#pragma once

#include <yangutil/yangavtype.h>
#include <yangrtc/YangPeerConnection2.h>

class WebRTCConfig
{
  public:

    static YangAVInfo* GetContextConfig()
    {
        YangContext *context = new YangContext();
        context->avinfo.audio.sample = 48000;
        context->avinfo.audio.channel = 2;
        // context->avinfo.rtc.

        return &context->avinfo;
    }
};
