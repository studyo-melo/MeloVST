#include <CDSPResampler.h>

#include "Rtc/OpusCodecWrapper.h"
#include <juce_core/juce_core.h>
#include <vector>

#include "Utils/AudioUtils.h"

class OpusCodecWrapperTest : public juce::UnitTest {
public:
    OpusCodecWrapperTest() : juce::UnitTest("OpusCodecWrapperTest") {}

    void runTest() override {
        testEncodingDecoding();
        // testInvalidInputs();
    }

private:
    void testEncodingDecoding() {
        beginTest("Encoding and Decoding");

        OpusCodecWrapper codec;

        // Simuler un tampon audio PCM (2 canaux, 48000 Hz, 20 ms de données)
        const int sampleRate = 48000;
        const int channels = 2;
        const int frameSize = static_cast<int>(0.02 * sampleRate); // 20 ms de données
        auto inbuf = generate_music(frameSize * channels);

        auto encodedData = codec.encode(inbuf);
        expect(!encodedData.empty(), "Encoded data should not be empty");
        expect(encodedData.size() > 0, "Encoded data size should be greater than 0");
        expect(encodedData.size() < inbuf.size(), "Encoded data size should be less than input data size");

        auto decodedData = codec.decode(encodedData);
        expect(!decodedData.empty(), "Decoded data should not be empty");
        expect(decodedData.size() > 0, "Decoded data size should be greater than 0");
        expect(decodedData.size() == inbuf.size(), "Decoded data size should be equal to input data size");
        expect(decodedData.size() > encodedData.size(), "Decoded data size should be greater than encoded data size");
        expectEquals(static_cast<int>(decodedData.size()), frameSize * channels, "Decoded data size mismatch");

        for (int i = 0; i < frameSize * channels; ++i) {
            expectWithinAbsoluteError(decodedData[i],  inbuf[i], 0.001f, "Decoded data mismatch");
        }
    }

    opus_uint32 Rz, Rw;
    OPUS_INLINE opus_uint32 fast_rand()
    {
        Rz=36969*(Rz&65535)+(Rz>>16);
        Rw=18000*(Rw&65535)+(Rw>>16);
        return (Rz<<16)+Rw;
    }

    std::vector<float> generate_music(const int len)
    {
        std::vector<float> buf = std::vector<float>(len, 0.0f);
        opus_int32 a1,b1,a2,b2;
        opus_int32 c1,c2,d1,d2;
        opus_int32 i,j;
        a1=b1=a2=b2=0;
        c1=c2=d1=d2=0;
        j=0;
        /*60ms silence*/
        for(i=0;i<2880;i++)buf[i*2]=buf[i*2+1]=0;
        for(i=2880;i<len;i++)
        {
            opus_uint32 r;
            opus_int32 v1,v2;
            v1=v2=(((j*((j>>12)^((j>>10|j>>12)&26&j>>7)))&128)+128)<<15;
            r=fast_rand();v1+=r&65535;v1-=r>>16;
            r=fast_rand();v2+=r&65535;v2-=r>>16;
            b1=v1-a1+((b1*61+32)>>6);a1=v1;
            b2=v2-a2+((b2*61+32)>>6);a2=v2;
            c1=(30*(c1+b1+d1)+32)>>6;d1=b1;
            c2=(30*(c2+b2+d2)+32)>>6;d2=b2;
            v1=(c1+128)>>8;
            v2=(c2+128)>>8;
            buf[i*2]=v1>32767?32767:(v1<-32768?-32768:v1);
            buf[i*2+1]=v2>32767?32767:(v2<-32768?-32768:v2);
            if(i%6==0)j++;
        }

        return buf;
    }


    // void testInvalidInputs() {
    //     beginTest("Invalid Inputs");
    //
    //     OpusCodecWrapper codec;
    //
    //     // Cas : Décoder des données invalides
    //     std::vector<uint8_t> invalidData = {0x00, 0x01, 0x02}; // Données non valides
    //     bool decodingErrorCaught = false;
    //     try {
    //         codec.decode(invalidData);
    //     } catch (const std::runtime_error& e) {
    //         decodingErrorCaught = true;
    //     }
    //     expect(decodingErrorCaught, "Decoding invalid data should throw an error");
    //
    //     // Cas : Encoder avec un pointeur nul
    //     bool encodingErrorCaught = false;
    //     try {
    //         codec.encode(nullptr);
    //     } catch (const std::runtime_error& e) {
    //         encodingErrorCaught = true;
    //     }
    //     expect(encodingErrorCaught, "Encoding with a null pointer should throw an error");
    // }
};

// Inscription du test
static OpusCodecWrapperTest opusCodecWrapperTest;
