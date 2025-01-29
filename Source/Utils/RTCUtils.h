#pragma once
#include <juce_core/juce_core.h>
#include <yangutil/yangavctype.h>

namespace RTCUtils {
   static juce::String getSignalingStateLabel(YangRequestType signalingState) {
    if (!signalingState) {
        return juce::String::fromUTF8("Inconnu");
    }
    switch (signalingState) {
        case Yang_Req_HighLostPacketRate:
            return juce::String::fromUTF8("Stable");
        case Yang_Req_Connected:
            return juce::String::fromUTF8("Connecté");
        case Yang_Req_Disconnected:
            return juce::String::fromUTF8("Déconnecté");
        case Yang_Req_Sendkeyframe:
            return juce::String::fromUTF8("Envoi de trames clés");
        case Yang_Req_LowLostPacketRate:
            return juce::String::fromUTF8("Faible taux de perte de paquets");
        default:
            return juce::String::fromUTF8("Inconnu");
    }
}

    static juce::String getIceCandidateStateLabel(YangIceCandidateState iceState) {
        if (!iceState) {
            return juce::String::fromUTF8("Inconnu");
        }
        switch (iceState) {
            case YangIceNew:
                return juce::String::fromUTF8("Nouveau");
            case YangIceSuccess:
                return juce::String::fromUTF8("Connecté");
            case YangIceFail:
                return juce::String::fromUTF8("Échoué");
            default:
                return juce::String::fromUTF8("Inconnu");
        }
    }
}