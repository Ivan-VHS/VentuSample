#include "dsp.h"
#include "resample.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double rms(const pl_audio_buffer *audio) {
    double sum = 0.0;
    const size_t count = audio->frames * audio->channels;
    for (size_t index = 0u; index < count; ++index) {
        const double value = (double)audio->samples[index];
        sum += value * value;
    }
    return count == 0u ? 0.0 : sqrt(sum / (double)count);
}

static void test_linear_preview_contract(void) {
    pl_audio_buffer input = {0};
    pl_audio_buffer output = {0};
    input.frames = 4u;
    input.sample_rate = 8000u;
    input.channels = 1u;
    input.samples = (float *)malloc(input.frames * sizeof(float));
    assert(input.samples != NULL);
    input.samples[0] = 0.0f;
    input.samples[1] = 1.0f;
    input.samples[2] = 0.0f;
    input.samples[3] = -1.0f;

    assert(ventu_audio_resample_linear(&input, 16000u, &output) == 0);
    assert(output.sample_rate == 16000u);
    assert(output.channels == 1u);
    assert(output.frames == 8u);
    assert(fabs((double)output.samples[1] - 0.5) < 1e-6);
    assert(ventu_audio_resample_linear(&input, 7999u, &(pl_audio_buffer){0}) != 0);

    pl_audio_free(&input);
    pl_audio_free(&output);
}

static void test_sinc_downsampling_rejects_aliasing(void) {
    pl_audio_buffer input = {0};
    pl_audio_buffer linear = {0};
    pl_audio_buffer sinc = {0};

    input.frames = 48000u;
    input.sample_rate = 48000u;
    input.channels = 1u;
    input.samples = (float *)malloc(input.frames * sizeof(float));
    assert(input.samples != NULL);

    /* 17 kHz is above the 6 kHz Nyquist limit of a 12 kHz output. */
    for (size_t frame = 0u; frame < input.frames; ++frame) {
        const double phase = 2.0 * 3.14159265358979323846 * 17000.0
            * (double)frame / (double)input.sample_rate;
        input.samples[frame] = (float)sin(phase);
    }

    assert(ventu_audio_resample_linear(&input, 12000u, &linear) == 0);
    assert(ventu_audio_resample_sinc(&input, 12000u, &sinc) == 0);
    assert(linear.frames == 12000u && sinc.frames == 12000u);
    assert(rms(&linear) > 0.60);
    assert(rms(&sinc) < rms(&linear) * 0.10);

    pl_audio_free(&input);
    pl_audio_free(&linear);
    pl_audio_free(&sinc);
}

static void test_sinc_preserves_passband_and_stereo_channels(void) {
    pl_audio_buffer input = {0};
    pl_audio_buffer output = {0};

    input.frames = 48000u;
    input.sample_rate = 48000u;
    input.channels = 1u;
    input.samples = (float *)malloc(input.frames * sizeof(float));
    assert(input.samples != NULL);

    for (size_t frame = 0u; frame < input.frames; ++frame) {
        const double phase = 2.0 * 3.14159265358979323846 * 1000.0
            * (double)frame / (double)input.sample_rate;
        input.samples[frame] = (float)sin(phase);
    }
    assert(ventu_audio_resample_sinc(&input, 12000u, &output) == 0);
    assert(fabs(rms(&output) - 0.70710678) < 0.03);
    pl_audio_free(&input);
    pl_audio_free(&output);

    input.frames = 4u;
    input.sample_rate = 48000u;
    input.channels = 2u;
    input.samples = (float *)malloc(input.frames * input.channels * sizeof(float));
    assert(input.samples != NULL);
    for (size_t frame = 0u; frame < input.frames; ++frame) {
        input.samples[frame * 2u] = 0.25f;
        input.samples[frame * 2u + 1u] = -0.5f;
    }
    assert(ventu_audio_resample_sinc(&input, 48000u, &output) == 0);
    assert(output.frames == input.frames && output.channels == 2u);
    for (size_t frame = 0u; frame < output.frames; ++frame) {
        assert(fabs((double)output.samples[frame * 2u] - 0.25) < 1e-5);
        assert(fabs((double)output.samples[frame * 2u + 1u] + 0.5) < 1e-5);
    }
    pl_audio_free(&input);
    pl_audio_free(&output);
}

static void test_sinc_rejects_invalid_inputs_without_publishing_output(void) {
    pl_audio_buffer input = {0};
    pl_audio_buffer output = {0};
    float sample = 0.0f;

    input.samples = &sample;
    input.frames = 1u;
    input.sample_rate = 48000u;
    input.channels = 1u;
    sample = NAN;
    assert(ventu_audio_resample_sinc(&input, 44100u, &output) != 0);
    assert(output.samples == NULL && output.frames == 0u);
    assert(output.sample_rate == 0u && output.channels == 0u);

    sample = 0.0f;
    output.sample_rate = 44100u;
    assert(ventu_audio_resample_sinc(&input, 44100u, &output) != 0);
    assert(output.samples == NULL && output.sample_rate == 44100u);
}

int main(void) {
    test_linear_preview_contract();
    test_sinc_downsampling_rejects_aliasing();
    test_sinc_preserves_passband_and_stereo_channels();
    test_sinc_rejects_invalid_inputs_without_publishing_output();
    return 0;
}
