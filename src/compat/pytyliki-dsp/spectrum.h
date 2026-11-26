#ifndef VENTUSAMPLE_SPECTRUM_H
#define VENTUSAMPLE_SPECTRUM_H

#include "dsp.h"

#include <limits.h>
#include <stddef.h>

#define PL_AUDIO_SPECTRUM_MIN_FFT_SIZE 256u
#define PL_AUDIO_SPECTRUM_MAX_FFT_SIZE 65536u
#define PL_AUDIO_SPECTRUM_MIX_CHANNELS UINT_MAX

typedef struct {
    size_t fft_size;
    size_t bin_count;
    unsigned sample_rate;
    double bin_width_hz;
    double peak_frequency_hz;
    double peak_magnitude;
    double spectral_centroid_hz;
    double rolloff_85_hz;
} pl_audio_spectrum_stats;

/*
 * Analyze the first fft_size frames of one channel or a mono mix.
 * Pass PL_AUDIO_SPECTRUM_MIX_CHANNELS to average all channels.
 *
 * fft_size must be a power of two in [256, 65536]. The caller supplies
 * fft_size / 2 + 1 output magnitudes. A Hann window is applied and magnitudes
 * are normalized by the window's coherent gain. This is an offline analysis
 * routine: it does not access an audio device or modify the input buffer.
 *
 * Returns 0 on success, -1 for invalid input/capacity, and -2 on allocation
 * failure. No output is published until the complete analysis succeeds.
 */
int pl_audio_analyze_spectrum(
    const pl_audio_buffer *audio,
    size_t fft_size,
    unsigned channel,
    double *magnitudes,
    size_t magnitude_capacity,
    pl_audio_spectrum_stats *stats
);

#endif
