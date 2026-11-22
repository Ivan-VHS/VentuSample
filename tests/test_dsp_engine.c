#include "ventu_block_engine.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    ventu_dsp_engine engine;
    ventu_dsp_biquad biquad;
    ventu_dsp_fir fir;
    ventu_dsp_limiter limiter;
    ventu_dsp_block_stats stats;
    float input[128];
    float output[128];
    float state[4];
    const float coefficients[4] = {0.25f, 0.25f, 0.25f, 0.25f};
    size_t i;

    assert(ventu_dsp_engine_init(&engine, 48000u, 128u) == 0);
    assert(strcmp(ventu_dsp_backend_name(engine.backend), "portable-c11-scalar") == 0);
    assert(ventu_dsp_engine_init(&engine, 1000u, 128u) != 0);

    for (i = 0u; i < 128u; ++i) {
        input[i] = (float)sin(2.0 * 3.141592653589793 * 440.0 * (double)i / 48000.0);
    }
    assert(ventu_dsp_biquad_lowpass_init(&biquad, 48000u, 5000.0, 0.707) == 0);
    assert(ventu_dsp_engine_biquad_process(&engine, &biquad, input, output, 128u) == 0);
    {
        double z1 = biquad.z1, z2 = biquad.z2;
        assert(ventu_dsp_engine_biquad_process(&engine, &biquad, input, output, 129u) != 0);
        assert(biquad.z1 == z1 && biquad.z2 == z2);
    }
    assert(ventu_dsp_measure_block(output, 128u, &stats) == 0 && stats.rms > 0.0);
    ventu_dsp_biquad_reset(&biquad);

    assert(ventu_dsp_fir_init(&fir, coefficients, 4u, state, 4u) == 0);
    assert(ventu_dsp_engine_fir_process(&engine, &fir, input, output, 128u) == 0);
    {
        size_t cursor = fir.cursor;
        float state0 = state[0];
        assert(ventu_dsp_engine_fir_process(&engine, &fir, input, output, 129u) != 0);
        assert(fir.cursor == cursor && state[0] == state0);
    }
    ventu_dsp_fir_reset(&fir);

    assert(ventu_dsp_limiter_init(&limiter, 48000u, -1.0, 0.1, 80.0) == 0);
    for (i = 0u; i < 128u; ++i) {
        input[i] = 1.5f;
    }
    assert(ventu_dsp_engine_limiter_process(&engine, &limiter, input, output, 128u) == 0);
    {
        double envelope = limiter.envelope;
        assert(ventu_dsp_engine_limiter_process(&engine, &limiter, input, output, 129u) != 0);
        assert(limiter.envelope == envelope);
    }
    assert(fabs((double)output[0]) <= limiter.threshold + 1e-6);
    assert(fabs((double)output[127]) <= limiter.threshold + 1e-6);

    /* A one-sample transient immediately after reset must never exceed the
     * configured sample ceiling, even while the smoothed envelope is low. */
    ventu_dsp_limiter_reset(&limiter);
    for (i = 0u; i < 128u; ++i) {
        input[i] = 0.0f;
    }
    input[0] = 2.0f;
    assert(ventu_dsp_engine_limiter_process(&engine, &limiter, input, output, 128u) == 0);
    assert(fabs((double)output[0]) <= limiter.threshold + 1e-6);
    for (i = 1u; i < 128u; ++i) {
        assert(fabs((double)output[i]) <= limiter.threshold + 1e-6);
    }

    input[0] = NAN;
    assert(ventu_dsp_measure_block(input, 128u, &stats) == 0 && stats.non_finite_samples == 1u);
    assert(ventu_dsp_engine_biquad_process(&engine, &biquad, input, output, 128u) != 0);
    return 0;
}
