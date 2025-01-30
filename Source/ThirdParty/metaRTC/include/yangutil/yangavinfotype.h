//
// Copyright (c) 2019-2022 yanggaofeng
//

#ifndef YANGUTIL_YANGAVINFOTYPE_H_
#define YANGUTIL_YANGAVINFOTYPE_H_
#include <yangutil/yangtype.h>
#include <yangutil/yangavtype.h>


//sample	int32_t	Fréquence d'échantillonnage (ex. 44100 Hz, 48000 Hz).
//frameSize	int32_t	Nombre d'échantillons par trame audio.
//bitrate	int32_t	Débit binaire de l’audio (ex. 128 kbps, 256 kbps).
//channel	int32_t	Nombre de canaux (1 = mono, 2 = stéréo).
//enableMono	yangbool	Active/désactive la conversion en mono.
//enableAec	yangbool	Active/désactive l'annulation d'écho (AEC).
//echoPath	int32_t	Longueur du chemin d'écho (en ms).
//enableAudioFec	yangbool	Active/désactive la correction d’erreur audio (FEC).
//aecBufferFrames	int32_t	Taille du buffer AEC (nombre de frames).
//audioCacheSize	int32_t	Taille du cache audio (en octets).
//audioCacheNum	int32_t	Nombre de buffers audio en cache.
//audioPlayCacheNum	int32_t	Nombre de buffers pour la lecture audio.
//enableAudioHeader	yangbool	Indique si l'audio inclut un en-tête.
//audioEncoderType	int32_t	Type d’encodeur audio utilisé (ex. Opus, AAC).
//audioDecoderType	int32_t	Type de décodeur audio utilisé.
//audioPlayType	int32_t	Mode de lecture audio.
//aIndex / aSubIndex	int32_t	Indexation spécifique au système audio.
typedef struct YangAudioInfo {
	int32_t sample;
	int32_t frameSize;
	int32_t bitrate;
	int32_t channel;

	yangbool enableMono;
	yangbool enableAec;
	int32_t echoPath;

	yangbool enableAudioFec;

	int32_t aecBufferFrames;
	int32_t audioCacheSize;
	int32_t audioCacheNum;
	int32_t audioPlayCacheNum;

	yangbool enableAudioHeader;
	int32_t audioEncoderType;
	int32_t audioDecoderType;
	int32_t audioPlayType;

	int32_t aIndex;
	int32_t aSubIndex;
}YangAudioInfo;

//width / height	int32_t	Résolution de la capture vidéo (ex. 1920x1080).
// outWidth / outHeight	int32_t	Résolution de sortie après traitement.
// rate	int32_t	Bitrate vidéo (ex. 512 kbps, 1000 kbps).
// frame	int32_t	Framerate (ex. 25 FPS, 30 FPS).
// rotate	int32_t	Rotation de l'image (ex. 0°, 90°, 180°).
// bitDepth	int32_t	Profondeur de couleur (ex. 8 bits, 10 bits).
// videoCacheNum	int32_t	Nombre de frames vidéo en cache.
// evideoCacheNum	int32_t	Cache supplémentaire pour l’encodage.
// videoPlayCacheNum	int32_t	Nombre de frames pour la lecture vidéo.
// videoCaptureFormat	YangColorSpace	Format vidéo capturé (ex. YUV, RGB).
// videoEncoderFormat	YangColorSpace	Format vidéo encodé.
// videoDecoderFormat	YangColorSpace	Format vidéo décodé.
// videoEncoderType	int32_t	Type d’encodeur vidéo (ex. H.264, VP8).
// videoDecoderType	int32_t	Type de décodeur vidéo.
// videoEncHwType	int32_t	Encodeur matériel utilisé (GPU, ASIC).
// videoDecHwType	int32_t	Décodeur matériel utilisé.
// vIndex	int32_t	Indexation pour le traitement vidéo.
typedef struct YangVideoInfo {
	int32_t width; //= 800
	int32_t height; //= 600
	int32_t outWidth;
	int32_t outHeight;
	int32_t rate; // 512
	int32_t frame; //25
	int32_t rotate; // 16
	int32_t bitDepth;

	int32_t videoCacheNum;
	int32_t evideoCacheNum;
	int32_t videoPlayCacheNum;

	YangColorSpace videoCaptureFormat;
	YangColorSpace videoEncoderFormat;
	YangColorSpace videoDecoderFormat;

	int32_t videoEncoderType;
	int32_t videoDecoderType;
	int32_t videoEncHwType;
	int32_t videoDecHwType;
	int32_t vIndex;
}YangVideoInfo;
//preset	int32_t	Préréglage de qualité (ex. ultrafast, slow).
// level_idc	int32_t	Niveau du profil H.264/H.265 (ex. 4.1, 5.1).
// profile	int32_t	Profil d’encodage (ex. baseline, high).
// keyint_max	int32_t	Intervalle entre images clés (GOP).
// enc_threads	int32_t	Nombre de threads pour l’encodage.
// gop	int32_t	Taille du GOP (ex. 60 = 2s à 30 FPS).
// createMeta	yangbool	Ajoute ou non les métadonnées.
typedef struct YangVideoEncInfo {
	int32_t preset;
	int32_t level_idc;
	int32_t profile;
	int32_t keyint_max;
	int32_t enc_threads;
	int32_t gop;
	yangbool createMeta;
}YangVideoEncInfo;

// familyType	YangIpFamilyType	Type d'IP (IPv4, IPv6).
// enableLogFile	yangbool	Active/désactive l’enregistrement des logs.
// mediaServer	int32_t	Type de serveur média utilisé.
// httpPort	int32_t	Port HTTP pour l’API.
// transType	int32_t	Type de transmission (TCP/UDP).
// logLevel	int32_t	Niveau de log (DEBUG, INFO, ERROR).
// whipUrl	char[128]	URL WHIP pour WebRTC Ingestion.
// whepUrl	char[128]	URL WHEP pour WebRTC Playback.
typedef struct YangSysInfo {
	YangIpFamilyType familyType;
	yangbool enableLogFile;
	int32_t mediaServer;
	int32_t httpPort;
	int32_t transType;
	int32_t logLevel;
	char whipUrl[128];
	char whepUrl[128];
}YangSysInfo;

// enableHttpServerSdp	yangbool	Active/désactive le serveur SDP HTTP.
// sessionTimeout	int32_t	Durée avant expiration d’une session.
// iceCandidateType	int32_t	Type de candidats ICE (host, srflx, relay).
// iceUsingLocalIp	yangbool	Force l’utilisation d’une IP locale.
// iceServerPort	int32_t	Port du serveur ICE.
// enableAudioBuffer	yangbool	Active un buffer pour l’audio.
// rtcSocketProtocol	int32_t	Protocole pour WebRTC (UDP/TCP).
// turnSocketProtocol	int32_t	Protocole TURN (UDP/TCP).
// rtcPort	int32_t	Port WebRTC principal.
// rtcLocalPort	int32_t	Port WebRTC local.
// localIp	char[32]	Adresse IP locale.
// rtcServerIP	char[32]	Adresse IP du serveur WebRTC.
// iceServerIP	char[64]	Adresse du serveur ICE.
// iceLocalIP	char[64]	Adresse locale ICE.
// iceUserName	char[32]	Nom d’utilisateur ICE.
// icePassword	char[64]	Mot de passe ICE.
typedef struct YangRtcInfo {
	yangbool enableHttpServerSdp;

	int32_t sessionTimeout;
        //yangbool enableDatachannel;
	int32_t iceCandidateType;
	yangbool iceUsingLocalIp;
	int32_t iceServerPort;
	yangbool enableAudioBuffer;
	int32_t rtcSocketProtocol;
	int32_t turnSocketProtocol;

	int32_t rtcPort;
	int32_t rtcLocalPort;

	char localIp[32];
	char rtcServerIP[32];

	char iceServerIP[64];
	char iceLocalIP[64];
	char iceUserName[32];
	char icePassword[64];
}YangRtcInfo;

typedef struct YangAVInfo{
	YangSysInfo sys;
	YangAudioInfo audio;
	YangVideoInfo video;
	YangVideoEncInfo enc;
	YangRtcInfo rtc;
}YangAVInfo;


#ifdef __cplusplus
#include <yangstream/YangStreamManager.h>
#include <yangstream/YangSynBufferManager.h>
class YangContext {
public:
	YangContext();
	virtual ~YangContext();
	void init(char *filename);
	void init();

	virtual void initExt(void *filename);
	virtual void initExt();
public:

	YangAVInfo avinfo;
	YangRtcCallback rtcCallback;
	YangSendRtcMessage sendRtcMessage;

#if Yang_OS_ANDROID
	void* nativeWindow;
#endif

        YangSynBufferManager synMgr;
        YangStreamManager* streams;
};
extern "C"{
void yang_init_avinfo(YangAVInfo* avinfo);
}
#else
void yang_init_avinfo(YangAVInfo* avinfo);
#endif

#endif /* YANGUTIL_YANGTYPE_H_ */
