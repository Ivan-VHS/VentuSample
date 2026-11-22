#ifndef VENTU_AUDIO_H
#define VENTU_AUDIO_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define VENTU_AUDIO_MAX_FRAMES 4800000u
typedef struct { float *samples; size_t frames; unsigned sample_rate; unsigned channels; } ventu_audio_buffer;
typedef struct { double peak; double rms; double dc; double crest_db; size_t clipped_samples; } ventu_audio_stats;
void ventu_audio_free(ventu_audio_buffer *audio);
int ventu_audio_generate_tone(ventu_audio_buffer *audio,unsigned sample_rate,unsigned channels,double frequency_hz,double seconds);
int ventu_audio_load_wav(const char *path,ventu_audio_buffer *audio);
int ventu_audio_write_wav(const char *path,const ventu_audio_buffer *audio);
int ventu_audio_apply_gain(ventu_audio_buffer *audio,double gain_db);
int ventu_audio_apply_lowpass(ventu_audio_buffer *audio,double cutoff_hz);
int ventu_audio_apply_compressor(ventu_audio_buffer *audio,double threshold_db,double ratio);
int ventu_audio_apply_saturation(ventu_audio_buffer *audio,double drive);
int ventu_audio_measure(const ventu_audio_buffer *audio,ventu_audio_stats *stats);
#ifdef __cplusplus
}
#endif

#endif
