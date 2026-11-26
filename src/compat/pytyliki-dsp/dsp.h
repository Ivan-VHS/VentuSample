#ifndef PYTYLIKI_DSP_H
#define PYTYLIKI_DSP_H
#include <stddef.h>
#include <stdint.h>
#define PL_AUDIO_MAX_FRAMES 4800000u
typedef struct { float *samples; size_t frames; unsigned sample_rate; unsigned channels; } pl_audio_buffer;
typedef struct { double peak; double rms; double dc; double crest_db; size_t clipped_samples; } pl_audio_stats;
void pl_audio_free(pl_audio_buffer *audio);
int pl_audio_generate_tone(pl_audio_buffer *audio,unsigned sample_rate,unsigned channels,double frequency_hz,double seconds);
int pl_audio_load_wav(const char *path,pl_audio_buffer *audio);
int pl_audio_write_wav(const char *path,const pl_audio_buffer *audio);
int pl_audio_apply_gain(pl_audio_buffer *audio,double gain_db);
int pl_audio_apply_lowpass(pl_audio_buffer *audio,double cutoff_hz);
int pl_audio_apply_compressor(pl_audio_buffer *audio,double threshold_db,double ratio);
int pl_audio_apply_saturation(pl_audio_buffer *audio,double drive);
int pl_audio_measure(const pl_audio_buffer *audio,pl_audio_stats *stats);
#endif
