#include "dsp.h"
#include "spectrum.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>

int main(void) {
    pl_audio_buffer audio = {0};
    pl_audio_spectrum_stats stats;
    const size_t fft_size = 4096u;
    const size_t bins = fft_size / 2u + 1u;
    double *magnitudes = (double *)calloc(bins, sizeof(*magnitudes));
    assert(magnitudes != NULL);

    assert(pl_audio_generate_tone(&audio, 48000u, 2u, 937.5, 4096.0 / 48000.0) == 0);
    assert(pl_audio_analyze_spectrum(
        &audio, fft_size, 0u, magnitudes, bins, &stats
    ) == 0);
    assert(stats.fft_size == fft_size);
    assert(stats.bin_count == bins);
    assert(fabs(stats.bin_width_hz - (48000.0 / 4096.0)) < 1e-12);
    assert(fabs(stats.peak_frequency_hz - 937.5) < 1e-8);
    assert(stats.peak_magnitude > 0.69 && stats.peak_magnitude < 0.71);
    assert(fabs(stats.spectral_centroid_hz - 937.5) < 0.5);
    assert(stats.rolloff_85_hz > 900.0 && stats.rolloff_85_hz < 980.0);

    assert(pl_audio_analyze_spectrum(
        &audio, fft_size, PL_AUDIO_SPECTRUM_MIX_CHANNELS,
        magnitudes, bins, &stats
    ) == 0);
    assert(fabs(stats.peak_frequency_hz - 937.5) < 1e-8);

    assert(pl_audio_analyze_spectrum(&audio, 3000u, 0u, magnitudes, bins, &stats) != 0);
    assert(pl_audio_analyze_spectrum(&audio, fft_size, 2u, magnitudes, bins, &stats) != 0);
    assert(pl_audio_analyze_spectrum(&audio, fft_size, 0u, magnitudes, bins - 1u, &stats) != 0);
    assert(pl_audio_analyze_spectrum(&audio, fft_size, 0u, magnitudes, bins, NULL) != 0);
    assert(pl_audio_analyze_spectrum(NULL, fft_size, 0u, magnitudes, bins, &stats) != 0);

    pl_audio_free(&audio);
    free(magnitudes);
    return 0;
}
