#ifndef VENTUSAMPLE_RESAMPLE_H
#define VENTUSAMPLE_RESAMPLE_H

#include "dsp.h"

/* Offline linear interpolation for preview-quality sample-rate conversion. */
int ventu_audio_resample_linear(
    const pl_audio_buffer *input,
    unsigned target_sample_rate,
    pl_audio_buffer *output
);

#endif
