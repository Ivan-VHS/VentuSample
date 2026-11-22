#ifndef PYTYLIKI_DSP_ENGINE_H
#define PYTYLIKI_DSP_ENGINE_H
#include <stddef.h>
typedef enum { PL_DSP_BACKEND_SCALAR = 0 } pl_dsp_backend;
typedef struct { pl_dsp_backend backend; unsigned sample_rate; unsigned max_block_frames; } pl_dsp_engine;
typedef struct { double b0,b1,b2,a1,a2,z1,z2; } pl_dsp_biquad;
typedef struct { const float *coefficients; float *state; size_t taps,cursor; } pl_dsp_fir;
typedef struct { double threshold,attack_coeff,release_coeff,envelope; } pl_dsp_limiter;
typedef struct { double peak,rms,dc; size_t non_finite_samples; } pl_dsp_block_stats;
int pl_dsp_engine_init(pl_dsp_engine *,unsigned,unsigned);
const char *pl_dsp_backend_name(pl_dsp_backend);
int pl_dsp_biquad_lowpass_init(pl_dsp_biquad *,unsigned,double,double);
void pl_dsp_biquad_reset(pl_dsp_biquad *);
/* Low-level offline kernels do not apply an engine block limit. */
int pl_dsp_biquad_process(pl_dsp_biquad *,const float *,float *,size_t);
int pl_dsp_fir_init(pl_dsp_fir *,const float *,size_t,float *,size_t);
void pl_dsp_fir_reset(pl_dsp_fir *);
int pl_dsp_fir_process(pl_dsp_fir *,const float *,float *,size_t);
int pl_dsp_limiter_init(pl_dsp_limiter *,unsigned,double,double,double);
void pl_dsp_limiter_reset(pl_dsp_limiter *);
int pl_dsp_limiter_process(pl_dsp_limiter *,const float *,float *,size_t);
/* Engine entry points fail closed when n exceeds max_block_frames. */
int pl_dsp_engine_biquad_process(const pl_dsp_engine *,pl_dsp_biquad *,const float *,float *,size_t);
int pl_dsp_engine_fir_process(const pl_dsp_engine *,pl_dsp_fir *,const float *,float *,size_t);
int pl_dsp_engine_limiter_process(const pl_dsp_engine *,pl_dsp_limiter *,const float *,float *,size_t);
int pl_dsp_measure_block(const float *,size_t,pl_dsp_block_stats *);
#endif
