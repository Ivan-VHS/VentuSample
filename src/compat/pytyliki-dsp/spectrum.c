#include "spectrum.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PL_SPECTRUM_PI 3.14159265358979323846264338327950288

static int power_of_two(size_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static void fft_forward(double *real, double *imaginary, size_t size) {
    size_t i;
    size_t j = 0u;

    for (i = 1u; i < size; ++i) {
        size_t bit = size >> 1u;
        while ((j & bit) != 0u) {
            j ^= bit;
            bit >>= 1u;
        }
        j ^= bit;
        if (i < j) {
            double temporary = real[i];
            real[i] = real[j];
            real[j] = temporary;
            temporary = imaginary[i];
            imaginary[i] = imaginary[j];
            imaginary[j] = temporary;
        }
    }

    for (size_t length = 2u; length <= size; length <<= 1u) {
        const double angle = -2.0 * PL_SPECTRUM_PI / (double)length;
        const double step_real = cos(angle);
        const double step_imaginary = sin(angle);
        const size_t half = length / 2u;

        for (i = 0u; i < size; i += length) {
            double weight_real = 1.0;
            double weight_imaginary = 0.0;

            for (j = 0u; j < half; ++j) {
                const size_t even = i + j;
                const size_t odd = even + half;
                const double odd_real = real[odd] * weight_real -
                    imaginary[odd] * weight_imaginary;
                const double odd_imaginary = real[odd] * weight_imaginary +
                    imaginary[odd] * weight_real;
                const double even_real = real[even];
                const double even_imaginary = imaginary[even];

                real[even] = even_real + odd_real;
                imaginary[even] = even_imaginary + odd_imaginary;
                real[odd] = even_real - odd_real;
                imaginary[odd] = even_imaginary - odd_imaginary;

                {
                    const double next_real = weight_real * step_real -
                        weight_imaginary * step_imaginary;
                    weight_imaginary = weight_real * step_imaginary +
                        weight_imaginary * step_real;
                    weight_real = next_real;
                }
            }
        }
    }
}

int pl_audio_analyze_spectrum(
    const pl_audio_buffer *audio,
    size_t fft_size,
    unsigned channel,
    double *magnitudes,
    size_t magnitude_capacity,
    pl_audio_spectrum_stats *stats
) {
    double *real = NULL;
    double *imaginary = NULL;
    double window_sum = 0.0;
    double magnitude_sum = 0.0;
    double weighted_frequency_sum = 0.0;
    double total_magnitude = 0.0;
    double peak_magnitude = 0.0;
    double peak_frequency = 0.0;
    double rolloff_frequency = 0.0;
    size_t bin_count;
    size_t rolloff_bin = 0u;
    pl_audio_spectrum_stats result;

    if (audio == NULL || audio->samples == NULL || magnitudes == NULL ||
        stats == NULL || audio->frames < fft_size ||
        audio->frames > (size_t)PL_AUDIO_MAX_FRAMES ||
        audio->sample_rate < 8000u || audio->sample_rate > 192000u ||
        (audio->channels != 1u && audio->channels != 2u) ||
        (channel != PL_AUDIO_SPECTRUM_MIX_CHANNELS && channel >= audio->channels) ||
        fft_size < PL_AUDIO_SPECTRUM_MIN_FFT_SIZE ||
        fft_size > PL_AUDIO_SPECTRUM_MAX_FFT_SIZE ||
        !power_of_two(fft_size)) {
        return -1;
    }

    bin_count = fft_size / 2u + 1u;
    if (magnitude_capacity < bin_count) return -1;

    real = (double *)calloc(fft_size, sizeof(*real));
    imaginary = (double *)calloc(fft_size, sizeof(*imaginary));
    if (real == NULL || imaginary == NULL) {
        free(real);
        free(imaginary);
        return -2;
    }

    for (size_t frame = 0u; frame < fft_size; ++frame) {
        double sample = 0.0;
        double window = 1.0;
        if (channel == PL_AUDIO_SPECTRUM_MIX_CHANNELS) {
            for (unsigned source = 0u; source < audio->channels; ++source) {
                sample += (double)audio->samples[frame * audio->channels + source];
            }
            sample /= (double)audio->channels;
        } else {
            sample = (double)audio->samples[frame * audio->channels + channel];
        }
        if (!isfinite(sample)) {
            free(real);
            free(imaginary);
            return -1;
        }
        if (fft_size > 1u) {
            window = 0.5 - 0.5 * cos(
                2.0 * PL_SPECTRUM_PI * (double)frame / (double)(fft_size - 1u)
            );
        }
        real[frame] = sample * window;
        window_sum += window;
    }

    if (!isfinite(window_sum) || window_sum <= 0.0) {
        free(real);
        free(imaginary);
        return -1;
    }

    fft_forward(real, imaginary, fft_size);

    for (size_t bin = 0u; bin < bin_count; ++bin) {
        const double divisor = (bin == 0u || bin == fft_size / 2u)
            ? window_sum : window_sum / 2.0;
        const double magnitude = hypot(real[bin], imaginary[bin]) / divisor;
        const double frequency = (double)bin * (double)audio->sample_rate /
            (double)fft_size;
        if (!isfinite(magnitude) || !isfinite(frequency)) {
            free(real);
            free(imaginary);
            return -1;
        }
        magnitudes[bin] = magnitude;
        magnitude_sum += magnitude;
        weighted_frequency_sum += frequency * magnitude;
        if (magnitude > peak_magnitude) {
            peak_magnitude = magnitude;
            peak_frequency = frequency;
        }
    }

    if (magnitude_sum > 0.0) {
        const double target = magnitude_sum * 0.85;
        for (size_t bin = 0u; bin < bin_count; ++bin) {
            total_magnitude += magnitudes[bin];
            if (total_magnitude >= target) {
                rolloff_bin = bin;
                break;
            }
        }
        rolloff_frequency = (double)rolloff_bin * (double)audio->sample_rate /
            (double)fft_size;
    }

    memset(&result, 0, sizeof(result));
    result.fft_size = fft_size;
    result.bin_count = bin_count;
    result.sample_rate = audio->sample_rate;
    result.bin_width_hz = (double)audio->sample_rate / (double)fft_size;
    result.peak_frequency_hz = peak_frequency;
    result.peak_magnitude = peak_magnitude;
    result.spectral_centroid_hz = magnitude_sum > 0.0
        ? weighted_frequency_sum / magnitude_sum : 0.0;
    result.rolloff_85_hz = rolloff_frequency;

    free(real);
    free(imaginary);
    *stats = result;
    return 0;
}
