#include "dsp_engine.h"
#include <float.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846
static int valid(const float *x,size_t n){size_t i;if(!x||!n)return 0;for(i=0;i<n;i++)if(!isfinite((double)x[i]))return 0;return 1;}
int pl_dsp_engine_init(pl_dsp_engine *e,unsigned rate,unsigned block){if(!e||rate<8000u||rate>384000u||!block||block>65536u)return -1;e->backend=PL_DSP_BACKEND_SCALAR;e->sample_rate=rate;e->max_block_frames=block;return 0;}
const char *pl_dsp_backend_name(pl_dsp_backend b){return b==PL_DSP_BACKEND_SCALAR?"portable-c11-scalar":"unknown";}
static int engine_accepts_block(const pl_dsp_engine *e,size_t n){
    return e && e->backend==PL_DSP_BACKEND_SCALAR &&
           e->sample_rate>=8000u && e->sample_rate<=384000u &&
           e->max_block_frames>0u && n>0u &&
           n<=(size_t)e->max_block_frames;
}
int pl_dsp_engine_biquad_process(const pl_dsp_engine *e,pl_dsp_biquad *f,const float *in,float *out,size_t n){
    if(!engine_accepts_block(e,n))return -1;
    return pl_dsp_biquad_process(f,in,out,n);
}
int pl_dsp_engine_fir_process(const pl_dsp_engine *e,pl_dsp_fir *f,const float *in,float *out,size_t n){
    if(!engine_accepts_block(e,n))return -1;
    return pl_dsp_fir_process(f,in,out,n);
}
int pl_dsp_engine_limiter_process(const pl_dsp_engine *e,pl_dsp_limiter *l,const float *in,float *out,size_t n){
    if(!engine_accepts_block(e,n))return -1;
    return pl_dsp_limiter_process(l,in,out,n);
}
int pl_dsp_biquad_lowpass_init(pl_dsp_biquad*f,unsigned rate,double hz,double q){double w,c,s,a,a0;if(!f||rate<8000u||rate>384000u||!isfinite(hz)||!isfinite(q)||hz<=0.0||hz>=rate*0.49||q<0.1||q>20.0)return -1;w=2.0*PI*hz/rate;c=cos(w);s=sin(w);a=s/(2.0*q);a0=1.0+a;f->b0=((1.0-c)*0.5)/a0;f->b1=(1.0-c)/a0;f->b2=f->b0;f->a1=(-2.0*c)/a0;f->a2=(1.0-a)/a0;f->z1=f->z2=0.0;return 0;}
void pl_dsp_biquad_reset(pl_dsp_biquad*f){if(f)f->z1=f->z2=0.0;}
int pl_dsp_biquad_process(pl_dsp_biquad*f,const float*in,float*out,size_t n){size_t i;double z1,z2;if(!f||!valid(in,n)||!out||!isfinite(f->b0)||!isfinite(f->b1)||!isfinite(f->b2)||!isfinite(f->a1)||!isfinite(f->a2)||!isfinite(f->z1)||!isfinite(f->z2))return -1;z1=f->z1;z2=f->z2;for(i=0;i<n;i++){double x=in[i],y=f->b0*x+z1,n1=f->b1*x-f->a1*y+z2,n2=f->b2*x-f->a2*y;if(!isfinite(y)||!isfinite(n1)||!isfinite(n2)||fabs(y)>FLT_MAX)return -2;out[i]=(float)y;z1=n1;z2=n2;}f->z1=z1;f->z2=z2;return 0;}
int pl_dsp_fir_init(pl_dsp_fir*f,const float*c,size_t taps,float*state,size_t cap){size_t i;if(!f||!c||!state||!taps||taps>4096u||cap<taps)return -1;for(i=0;i<taps;i++)if(!isfinite((double)c[i]))return -1;f->coefficients=c;f->state=state;f->taps=taps;f->cursor=0;memset(state,0,taps*sizeof(*state));return 0;}
void pl_dsp_fir_reset(pl_dsp_fir*f){if(f&&f->state&&f->taps&&f->taps<=4096u){memset(f->state,0,f->taps*sizeof(*f->state));f->cursor=0;}}
int pl_dsp_fir_process(pl_dsp_fir*f,const float*in,float*out,size_t n){size_t i,k;if(!f||!f->coefficients||!f->state||!f->taps||f->taps>4096u||f->cursor>=f->taps||!valid(in,n)||!out)return -1;for(i=0;i<n;i++){double acc=0.0;f->state[f->cursor]=in[i];for(k=0;k<f->taps;k++){size_t p=(f->cursor+f->taps-k)%f->taps;acc+=(double)f->coefficients[k]*f->state[p];}if(!isfinite(acc)||fabs(acc)>FLT_MAX)return -2;out[i]=(float)acc;f->cursor=(f->cursor+1u)%f->taps;}return 0;}
int pl_dsp_limiter_init(pl_dsp_limiter*l,unsigned rate,double db,double attack,double release){if(!l||rate<8000u||rate>384000u||!isfinite(db)||!isfinite(attack)||!isfinite(release)||db< -60.0||db>0.0||attack<0.1||attack>100.0||release<1.0||release>2000.0)return -1;l->threshold=pow(10.0,db/20.0);l->attack_coeff=exp(-1.0/(0.001*attack*rate));l->release_coeff=exp(-1.0/(0.001*release*rate));l->envelope=0.0;return 0;}
void pl_dsp_limiter_reset(pl_dsp_limiter*l){if(l)l->envelope=0.0;}
int pl_dsp_limiter_process(pl_dsp_limiter*l,const float*in,float*out,size_t n){size_t i;double env;if(!l||!valid(in,n)||!out||!isfinite(l->threshold)||!isfinite(l->attack_coeff)||!isfinite(l->release_coeff)||!isfinite(l->envelope)||l->threshold<=0.0||l->attack_coeff<0.0||l->attack_coeff>=1.0||l->release_coeff<0.0||l->release_coeff>=1.0)return -1;env=l->envelope;for(i=0;i<n;i++){double level=fabs((double)in[i]),coef=level>env?l->attack_coeff:l->release_coeff;double gain;env=coef*env+(1.0-coef)*level;gain=env>l->threshold?l->threshold/env:1.0;if(level>l->threshold&&gain>l->threshold/level)gain=l->threshold/level;out[i]=(float)((double)in[i]*gain);}l->envelope=env;return 0;}
int pl_dsp_measure_block(const float*x,size_t n,pl_dsp_block_stats*s){size_t i,valid_n=0;double sum=0.0,sq=0.0;if(!x||!n||!s)return -1;memset(s,0,sizeof(*s));for(i=0;i<n;i++){double v=x[i],a;if(!isfinite(v)){s->non_finite_samples++;continue;}valid_n++;a=fabs(v);if(a>s->peak)s->peak=a;sum+=v;sq+=v*v;}if(!valid_n)return -2;s->dc=sum/valid_n;s->rms=sqrt(sq/valid_n);return 0;}
