#include "resample.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VENTU_MIN_SAMPLE_RATE 8000u
#define VENTU_MAX_SAMPLE_RATE 192000u

int ventu_audio_resample_linear(
    const pl_audio_buffer *input,
    unsigned target_sample_rate,
    pl_audio_buffer *output
) {
    pl_audio_buffer converted = {0};
    double frame_ratio;
    double rounded_frames;
    size_t output_frames;
    size_t output_samples;

    if (input == NULL || output == NULL || input == output ||
        input->samples == NULL || input->frames == 0u ||
        input->frames > (size_t)PL_AUDIO_MAX_FRAMES ||
        (input->channels != 1u && input->channels != 2u) ||
        input->sample_rate < VENTU_MIN_SAMPLE_RATE ||
        input->sample_rate > VENTU_MAX_SAMPLE_RATE ||
        target_sample_rate < VENTU_MIN_SAMPLE_RATE ||
        target_sample_rate > VENTU_MAX_SAMPLE_RATE ||
        output->samples != NULL || output->frames != 0u ||
        output->sample_rate != 0u || output->channels != 0u) {
        return -1;
    }

    for (size_t index = 0u; index < input->frames * input->channels; ++index) {
        if (!isfinite((double)input->samples[index])) {
            return -1;
        }
    }

    frame_ratio = (double)target_sample_rate / (double)input->sample_rate;
    rounded_frames = floor((double)input->frames * frame_ratio + 0.5);
    if (!isfinite(rounded_frames) || rounded_frames < 1.0 ||
        rounded_frames > (double)PL_AUDIO_MAX_FRAMES) {
        return -1;
    }

    output_frames = (size_t)rounded_frames;
    if (output_frames > SIZE_MAX / input->channels) {
        return -1;
    }
    output_samples = output_frames * input->channels;
    if (output_samples > SIZE_MAX / sizeof(*converted.samples)) {
        return -1;
    }

    converted.samples = (float *)malloc(output_samples * sizeof(*converted.samples));
    if (converted.samples == NULL) {
        return -2;
    }
    converted.frames = output_frames;
    converted.sample_rate = target_sample_rate;
    converted.channels = input->channels;

    for (size_t frame = 0u; frame < output_frames; ++frame) {
        const double source_position = (double)frame *
            (double)input->sample_rate / (double)target_sample_rate;
        size_t first = (size_t)source_position;
        double fraction = source_position - (double)first;
        size_t second;

        if (first >= input->frames - 1u) {
            first = input->frames - 1u;
            second = first;
            fraction = 0.0;
        } else {
            second = first + 1u;
        }

        for (unsigned channel = 0u; channel < input->channels; ++channel) {
            const size_t first_index = first * input->channels + channel;
            const size_t second_index = second * input->channels + channel;
            const double left = (double)input->samples[first_index];
            const double right = (double)input->samples[second_index];
            const double value = left + (right - left) * fraction;
            if (!isfinite(value)) {
                pl_audio_free(&converted);
                return -1;
            }
            converted.samples[frame * input->channels + channel] = (float)value;
        }
    }

    *output = converted;
    return 0;
}
