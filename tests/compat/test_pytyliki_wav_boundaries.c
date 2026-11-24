#include "dsp.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void write_u16(FILE *file, uint16_t value) {
    fputc((int)(value & 0xffu), file);
    fputc((int)(value >> 8), file);
}

static void write_u32(FILE *file, uint32_t value) {
    write_u16(file, (uint16_t)(value & 0xffffu));
    write_u16(file, (uint16_t)(value >> 16));
}

static FILE *open_fixture(const char *path) {
#if defined(_MSC_VER)
    FILE *file = NULL;
    if (fopen_s(&file, path, "wb") != 0) return NULL;
    return file;
#else
    return fopen(path, "wb");
#endif
}

static int write_fixture(const char *path, uint32_t riff_size,
                         uint32_t data_size, size_t actual_data_bytes) {
    FILE *file = open_fixture(path);
    if (file == NULL) return -1;
    fwrite("RIFF", 1u, 4u, file);
    write_u32(file, riff_size);
    fwrite("WAVEfmt ", 1u, 8u, file);
    write_u32(file, 16u);
    write_u16(file, 1u);
    write_u16(file, 1u);
    write_u32(file, 48000u);
    write_u32(file, 96000u);
    write_u16(file, 2u);
    write_u16(file, 16u);
    fwrite("data", 1u, 4u, file);
    write_u32(file, data_size);
    for (size_t i = 0u; i < actual_data_bytes; ++i) fputc(0, file);
    if (actual_data_bytes == data_size && (data_size & 1u)) fputc(0, file);
    if (fclose(file) != 0) return -1;
    return 0;
}

int main(void) {
    const char *path = "pytyliki_wav_boundary_fixture.wav";
    pl_audio_buffer audio = {0};

    assert(write_fixture(path, 38u, 2u, 2u) == 0);
    assert(pl_audio_load_wav(path, &audio) == 0);
    assert(audio.frames == 1u && audio.channels == 1u && audio.sample_rate == 48000u);
    pl_audio_free(&audio);

    /* The file has bytes after the declared RIFF end; they are not part of WAVE. */
    assert(write_fixture(path, 36u, 2u, 2u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    /* A chunk may not claim more bytes than remain in its RIFF container. */
    assert(write_fixture(path, 38u, 4u, 4u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    /* The RIFF extent itself must be present in the physical file. */
    assert(write_fixture(path, 38u, 2u, 1u) == 0);
    assert(pl_audio_load_wav(path, &audio) != 0);
    pl_audio_free(&audio);

    remove(path);
    puts("WAV RIFF container boundary tests passed.");
    return 0;
}
