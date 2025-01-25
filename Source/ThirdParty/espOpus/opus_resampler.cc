#include "opus_resampler.h"
#include "silk_resampler.h"

#define TAG "OpusResampler"

OpusResampler::OpusResampler() {
}

OpusResampler::~OpusResampler() {
}

void OpusResampler::Configure(int input_sample_rate, int output_sample_rate) {
    int encode = input_sample_rate > output_sample_rate ? 1 : 0;
    auto ret = silk_resampler_init(&resampler_state_, input_sample_rate, output_sample_rate, encode);
    if (ret != 0) {
        return;
    }
    input_sample_rate_ = input_sample_rate;
    output_sample_rate_ = output_sample_rate;
}

void OpusResampler::Process(const int16_t *input, int input_samples, int16_t *output) {
    auto ret = silk_resampler(&resampler_state_, output, input, input_samples);
    if (ret != 0) {
        return;
    }
}

int OpusResampler::GetOutputSamples(int input_samples) const {
    return input_samples * output_sample_rate_ / input_sample_rate_;
}
