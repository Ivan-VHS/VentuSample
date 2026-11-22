#ifndef VENTU_BLOCK_ENGINE_H
#define VENTU_BLOCK_ENGINE_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { VENTU_DSP_BACKEND_SCALAR = 0 } ventu_dsp_backend;
typedef struct { ventu_dsp_backend backend; unsigned sample_rate; unsigned max_block_frames; } ventu_dsp_engine;
typedef struct { double b0,b1,b2,a1,a2,z1,z2; } ventu_dsp_biquad;
typedef struct { const float *coefficients; float *state; size_t taps,cursor; } ventu_dsp_fir;
typedef struct { double threshold,attack_coeff,release_coeff,envelope; } ventu_dsp_limiter;
typedef struct { double peak,rms,dc; size_t non_finite_samples; } ventu_dsp_block_stats;
int ventu_dsp_engine_init(ventu_dsp_engine *,unsigned,unsigned);
const char *ventu_dsp_backend_name(ventu_dsp_backend);
int ventu_dsp_biquad_lowpass_init(ventu_dsp_biquad *,unsigned,double,double);
void ventu_dsp_biquad_reset(ventu_dsp_biquad *);
/* Low-level offline kernels do not apply an engine block limit. */
int ventu_dsp_biquad_process(ventu_dsp_biquad *,const float *,float *,size_t);
int ventu_dsp_fir_init(ventu_dsp_fir *,const float *,size_t,float *,size_t);
void ventu_dsp_fir_reset(ventu_dsp_fir *);
int ventu_dsp_fir_process(ventu_dsp_fir *,const float *,float *,size_t);
int ventu_dsp_limiter_init(ventu_dsp_limiter *,unsigned,double,double,double);
void ventu_dsp_limiter_reset(ventu_dsp_limiter *);
int ventu_dsp_limiter_process(ventu_dsp_limiter *,const float *,float *,size_t);
/* Engine entry points fail closed when n exceeds max_block_frames. */
int ventu_dsp_engine_biquad_process(const ventu_dsp_engine *,ventu_dsp_biquad *,const float *,float *,size_t);
int ventu_dsp_engine_fir_process(const ventu_dsp_engine *,ventu_dsp_fir *,const float *,float *,size_t);
int ventu_dsp_engine_limiter_process(const ventu_dsp_engine *,ventu_dsp_limiter *,const float *,float *,size_t);
int ventu_dsp_measure_block(const float *,size_t,ventu_dsp_block_stats *);
#ifdef __cplusplus
}
#endif

#endif
