#ifndef VENTUSAMPLE_AUDIO_ERRORS_H
#define VENTUSAMPLE_AUDIO_ERRORS_H

#include <stddef.h>
#include <stdio.h>

typedef struct {
    const char *code;
    const char *severity;
    const char *status;
    const char *title;
    const char *summary;
    const char *causes;
    const char *remedy;
    const char *documentation;
} ventu_audio_error;

size_t ventu_audio_error_count(void);
const ventu_audio_error *ventu_audio_error_at(size_t index);
const ventu_audio_error *ventu_audio_error_find(const char *code);
void ventu_audio_error_print(FILE *stream, const ventu_audio_error *entry);

#endif
