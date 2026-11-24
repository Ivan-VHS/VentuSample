#include "dsp.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PL_WAV_MAX_BYTES (32u*1024u*1024u)
#define PL_PI 3.14159265358979323846
static int audio_valid(const pl_audio_buffer*a){if(!a||!a->samples||a->frames==0||a->frames>PL_AUDIO_MAX_FRAMES||(a->channels!=1&&a->channels!=2)||a->sample_rate<8000||a->sample_rate>192000)return 0;size_t n=a->frames*a->channels;for(size_t i=0;i<n;i++)if(!isfinite((double)a->samples[i]))return 0;return 1;}
static uint16_t read_u16(const unsigned char*p){return(uint16_t)(p[0]|((uint16_t)p[1]<<8));}
static uint32_t read_u32(const unsigned char*p){return(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void write_u16(FILE*f,uint16_t v){fputc((int)(v&255u),f);fputc((int)(v>>8),f);}
static void write_u32(FILE*f,uint32_t v){write_u16(f,(uint16_t)(v&65535u));write_u16(f,(uint16_t)(v>>16));}
static FILE *open_binary_file(const char *path, const char *mode) {
#if defined(_MSC_VER)
    FILE *file = NULL;
    if (fopen_s(&file, path, mode) != 0) return NULL;
    return file;
#else
    return fopen(path, mode);
#endif
}
void pl_audio_free(pl_audio_buffer*a){if(!a)return;free(a->samples);memset(a,0,sizeof(*a));}
int pl_audio_generate_tone(pl_audio_buffer*a,unsigned rate,unsigned channels,double hz,double seconds){if(!a||rate<8000||rate>192000||(channels!=1&&channels!=2)||!isfinite(hz)||!isfinite(seconds)||hz<=0.0||hz>=(double)rate*0.5||seconds<=0.0||seconds>30.0)return -1;double frame_count=(double)rate*seconds;if(!isfinite(frame_count)||frame_count<1.0||frame_count>(double)PL_AUDIO_MAX_FRAMES)return -1;size_t frames=(size_t)frame_count;if(frames==0u||frames>PL_AUDIO_MAX_FRAMES)return -1;float*data=(float*)malloc(frames*(size_t)channels*sizeof(float));if(!data)return -2;for(size_t i=0;i<frames;i++){float s=(float)(0.7*sin(2.0*PL_PI*hz*(double)i/(double)rate));for(unsigned c=0;c<channels;c++)data[i*(size_t)channels+c]=s;}pl_audio_free(a);a->samples=data;a->frames=frames;a->sample_rate=rate;a->channels=channels;return 0;}
int pl_audio_load_wav(const char *path, pl_audio_buffer *audio) {
    FILE *file;
    unsigned char header[12];
    uint32_t riff_size;
    uint64_t riff_end;
    long file_size;
    unsigned format = 0u, channels = 0u, rate = 0u, bits = 0u, align = 0u;
    unsigned char *pcm = NULL;
    size_t data_size = 0u;
    int result = -2;

    if (path == NULL || audio == NULL) return -1;
    file = open_binary_file(path, "rb");
    if (file == NULL) return -1;

    if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) < 0L ||
        fseek(file, 0L, SEEK_SET) != 0 || file_size < 12L) {
        fclose(file);
        return -2;
    }
    if (fread(header, 1u, sizeof(header), file) != sizeof(header) ||
        memcmp(header, "RIFF", 4u) != 0 ||
        memcmp(header + 8u, "WAVE", 4u) != 0) {
        fclose(file);
        return -2;
    }

    riff_size = read_u32(header + 4u);
    riff_end = UINT64_C(8) + (uint64_t)riff_size;
    if (riff_size < 4u || riff_size > PL_WAV_MAX_BYTES + 4u ||
        riff_end > (uint64_t)file_size || fseek(file, 12L, SEEK_SET) != 0) {
        fclose(file);
        return -2;
    }

    while (1) {
        unsigned char chunk[8];
        uint32_t chunk_size;
        uint64_t padded_size;
        long chunk_position = ftell(file);

        if (chunk_position < 0L || (uint64_t)chunk_position > riff_end) goto malformed;
        if ((uint64_t)chunk_position == riff_end) break;
        if (riff_end - (uint64_t)chunk_position < sizeof(chunk) ||
            fread(chunk, 1u, sizeof(chunk), file) != sizeof(chunk)) goto malformed;

        chunk_size = read_u32(chunk + 4u);
        padded_size = (uint64_t)chunk_size + (uint64_t)(chunk_size & 1u);
        chunk_position = ftell(file);
        if (chunk_position < 0L || (uint64_t)chunk_position > riff_end ||
            padded_size > riff_end - (uint64_t)chunk_position) goto malformed;
        if (chunk_size > PL_WAV_MAX_BYTES) {
            result = -3;
            goto cleanup;
        }

        if (memcmp(chunk, "fmt ", 4u) == 0) {
            unsigned char fmt[40];
            size_t take = chunk_size < sizeof(fmt) ? (size_t)chunk_size : sizeof(fmt);
            if (format != 0u || take < 16u ||
                fread(fmt, 1u, take, file) != take) goto malformed;
            format = read_u16(fmt);
            channels = read_u16(fmt + 2u);
            rate = read_u32(fmt + 4u);
            align = read_u16(fmt + 12u);
            bits = read_u16(fmt + 14u);
            if (chunk_size > take &&
                fseek(file, (long)(chunk_size - (uint32_t)take), SEEK_CUR) != 0) goto malformed;
        } else if (memcmp(chunk, "data", 4u) == 0) {
            if (pcm != NULL) goto malformed;
            pcm = (unsigned char *)malloc(chunk_size != 0u ? (size_t)chunk_size : 1u);
            if (pcm == NULL) {
                result = -4;
                goto cleanup;
            }
            if (fread(pcm, 1u, chunk_size, file) != (size_t)chunk_size) goto malformed;
            data_size = (size_t)chunk_size;
        } else if (fseek(file, (long)chunk_size, SEEK_CUR) != 0) {
            goto malformed;
        }

        if ((chunk_size & 1u) != 0u && fgetc(file) == EOF) goto malformed;
        if (format != 0u && pcm != NULL) break;
    }

    if (fclose(file) != 0) {
        free(pcm);
        return -2;
    }
    file = NULL;
    if (format != 1u || bits != 16u || (channels != 1u && channels != 2u) ||
        rate < 8000u || rate > 192000u || align != channels * 2u ||
        pcm == NULL || data_size == 0u || data_size % align != 0u) {
        free(pcm);
        return -5;
    }

    {
        size_t frames = data_size / align;
        float *samples;
        if (frames == 0u || frames > PL_AUDIO_MAX_FRAMES) {
            free(pcm);
            return -3;
        }
        samples = (float *)malloc(frames * (size_t)channels * sizeof(float));
        if (samples == NULL) {
            free(pcm);
            return -4;
        }
        for (size_t i = 0u; i < frames * (size_t)channels; ++i) {
            int16_t value = (int16_t)read_u16(pcm + i * 2u);
            samples[i] = (float)((double)value / 32768.0);
        }
        free(pcm);
        pl_audio_free(audio);
        audio->samples = samples;
        audio->frames = frames;
        audio->sample_rate = rate;
        audio->channels = channels;
    }
    return 0;

malformed:
    result = -2;
cleanup:
    free(pcm);
    if (file != NULL) fclose(file);
    return result;
}
int pl_audio_write_wav(const char*path,const pl_audio_buffer*a){if(!path||!audio_valid(a))return -1;size_t count=a->frames*a->channels,data_bytes=count*2u;if(data_bytes>UINT32_MAX-36u)return -2;FILE*f=open_binary_file(path,"wb");if(!f)return -1;fwrite("RIFF",1,4,f);write_u32(f,(uint32_t)(36u+data_bytes));fwrite("WAVEfmt ",1,8,f);write_u32(f,16);write_u16(f,1);write_u16(f,(uint16_t)a->channels);write_u32(f,a->sample_rate);write_u32(f,a->sample_rate*a->channels*2u);write_u16(f,(uint16_t)(a->channels*2u));write_u16(f,16);fwrite("data",1,4,f);write_u32(f,(uint32_t)data_bytes);for(size_t i=0;i<count;i++){double x=a->samples[i];if(x>1.0)x=1.0;if(x< -1.0)x=-1.0;long v=(long)lrint(x*32767.0);if(v>32767)v=32767;if(v< -32768)v=-32768;write_u16(f,(uint16_t)(int16_t)v);}int bad=ferror(f);if(fclose(f)!=0)bad=1;return bad?-1:0;}
int pl_audio_apply_gain(pl_audio_buffer*a,double db){if(!audio_valid(a)||!isfinite(db)||db< -60.0||db>24.0)return -1;double g=pow(10.0,db/20.0);size_t n=a->frames*a->channels;for(size_t i=0;i<n;i++){double y=(double)a->samples[i]*g;if(!isfinite(y)||fabs(y)>(double)FLT_MAX)return -1;}for(size_t i=0;i<n;i++)a->samples[i]=(float)((double)a->samples[i]*g);return 0;}
int pl_audio_apply_lowpass(pl_audio_buffer*a,double cutoff){if(!audio_valid(a)||!isfinite(cutoff)||cutoff<10.0||cutoff>=a->sample_rate*0.49)return -1;double alpha=1.0-exp(-2.0*PL_PI*cutoff/(double)a->sample_rate);for(unsigned c=0;c<a->channels;c++){double y=0.0;for(size_t i=0;i<a->frames;i++){size_t k=i*a->channels+c;y+=alpha*((double)a->samples[k]-y);a->samples[k]=(float)y;}}return 0;}
int pl_audio_apply_compressor(pl_audio_buffer*a,double db,double ratio){if(!audio_valid(a)||!isfinite(db)||!isfinite(ratio)||db< -60.0||db>0.0||ratio<1.0||ratio>20.0)return -1;double threshold=pow(10.0,db/20.0);size_t n=a->frames*a->channels;for(size_t i=0;i<n;i++){double x=a->samples[i],ax=fabs(x);if(ax>threshold){double compressed=threshold*pow(ax/threshold,1.0/ratio);a->samples[i]=(float)copysign(compressed,x);}}return 0;}
int pl_audio_apply_saturation(pl_audio_buffer*a,double drive){if(!audio_valid(a)||!isfinite(drive)||drive<0.1||drive>12.0)return -1;double norm=tanh(drive);size_t n=a->frames*a->channels;for(size_t i=0;i<n;i++)a->samples[i]=(float)(tanh((double)a->samples[i]*drive)/norm);return 0;}
int pl_audio_measure(const pl_audio_buffer*a,pl_audio_stats*s){if(!audio_valid(a)||!s)return -1;memset(s,0,sizeof(*s));size_t n=a->frames*a->channels;double sum=0.0,squares=0.0;for(size_t i=0;i<n;i++){double x=a->samples[i],ax=fabs(x);if(ax>s->peak)s->peak=ax;if(ax>=1.0)s->clipped_samples++;sum+=x;squares+=x*x;}s->dc=sum/(double)n;s->rms=sqrt(squares/(double)n);s->crest_db=s->rms>0.0?20.0*log10(s->peak/s->rms):0.0;return 0;}
