#include "dsp.h"
#include "resample.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>

int main(void) {
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
    return 0;
}
