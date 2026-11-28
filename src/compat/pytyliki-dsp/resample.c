#include "resample.h"

#include <math.h>
#include <float.h>
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


#define VENTU_SINC_RADIUS 8u

static double sinc_pi(double value) {
    if (fabs(value) < 1e-12) return 1.0;
    return sin(3.14159265358979323846 * value) /
        (3.14159265358979323846 * value);
}

int ventu_audio_resample_sinc(
    const pl_audio_buffer *input,
    unsigned target_sample_rate,
    pl_audio_buffer *output
) {
    pl_audio_buffer converted = {0};
    double frame_ratio;
    double rounded_frames;
    double cutoff;
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
        if (!isfinite((double)input->samples[index])) return -1;
    }

    frame_ratio = (double)target_sample_rate / (double)input->sample_rate;
    rounded_frames = floor((double)input->frames * frame_ratio + 0.5);
    if (!isfinite(rounded_frames) || rounded_frames < 1.0 ||
        rounded_frames > (double)PL_AUDIO_MAX_FRAMES) {
        return -1;
    }

    output_frames = (size_t)rounded_frames;
    if (output_frames > SIZE_MAX / input->channels) return -1;
    output_samples = output_frames * input->channels;
    if (output_samples > SIZE_MAX / sizeof(*converted.samples)) return -1;

    if (target_sample_rate == input->sample_rate) {
        converted.samples = (float *)malloc(
            output_samples * sizeof(*converted.samples)
        );
        if (converted.samples == NULL) return -2;
        memcpy(
            converted.samples,
            input->samples,
            output_samples * sizeof(*converted.samples)
        );
        converted.frames = output_frames;
        converted.sample_rate = target_sample_rate;
        converted.channels = input->channels;
        *output = converted;
        return 0;
    }

    converted.samples = (float *)malloc(output_samples * sizeof(*converted.samples));
    if (converted.samples == NULL) return -2;
    converted.frames = output_frames;
    converted.sample_rate = target_sample_rate;
    converted.channels = input->channels;

    /* A lower target rate needs a proportionally lower anti-alias cutoff. */
    cutoff = frame_ratio < 1.0 ? frame_ratio * 0.95 : 1.0;

    for (size_t frame = 0u; frame < output_frames; ++frame) {
        const double source_position = (double)frame / frame_ratio;
        const long long center = (long long)floor(source_position);
        const long long first = center - (long long)VENTU_SINC_RADIUS + 1LL;

        for (unsigned channel = 0u; channel < input->channels; ++channel) {
            double weighted_sum = 0.0;
            double weight_sum = 0.0;

            for (unsigned tap = 0u; tap < VENTU_SINC_RADIUS * 2u; ++tap) {
                const long long source_index = first + (long long)tap;
                const double distance = (double)source_index - source_position;
                const double normalized_distance = distance / (double)VENTU_SINC_RADIUS;
                double window;
                double weight;
                size_t bounded_index;
                double sample;

                if (fabs(normalized_distance) >= 1.0) continue;

                /* Blackman window keeps the finite sinc kernel well behaved. */
                window = 0.42
                    + 0.50 * cos(3.14159265358979323846 * normalized_distance)
                    + 0.08 * cos(2.0 * 3.14159265358979323846 * normalized_distance);
                weight = cutoff * sinc_pi(cutoff * distance) * window;

                if (source_index < 0LL) {
                    bounded_index = 0u;
                } else if ((unsigned long long)source_index >=
                           (unsigned long long)input->frames) {
                    bounded_index = input->frames - 1u;
                } else {
                    bounded_index = (size_t)source_index;
                }

                sample = (double)input->samples[bounded_index * input->channels + channel];
                weighted_sum += sample * weight;
                weight_sum += weight;
            }

            if (!isfinite(weighted_sum) || !isfinite(weight_sum) ||
                fabs(weight_sum) < 1e-12) {
                pl_audio_free(&converted);
                return -1;
            }

            {
                const double value = weighted_sum / weight_sum;
                if (!isfinite(value) || value > (double)FLT_MAX ||
                    value < -(double)FLT_MAX) {
                    pl_audio_free(&converted);
                    return -1;
                }
                converted.samples[frame * input->channels + channel] = (float)value;
            }
        }
    }

    *output = converted;
    return 0;
}
