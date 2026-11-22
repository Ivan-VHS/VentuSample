#include "audio_errors.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const ventu_audio_error *entry;
    FILE *stream;
    char output[2048] = {0};
    size_t bytes;

    assert(ventu_audio_error_count() == 3u);
    assert(ventu_audio_error_at(0u) != NULL);
    assert(ventu_audio_error_at(ventu_audio_error_count()) == NULL);
    assert(ventu_audio_error_find(NULL) == NULL);
    assert(ventu_audio_error_find("VNT-A9999") == NULL);

    entry = ventu_audio_error_find("VNT-A0107");
    assert(entry != NULL);
    assert(strcmp(entry->severity, "error") == 0);
    assert(strstr(entry->remedy, "0.49") != NULL);

#if defined(_MSC_VER)
    assert(tmpfile_s(&stream) == 0);
#else
    stream = tmpfile();
    assert(stream != NULL);
#endif

    ventu_audio_error_print(stream, entry);
    rewind(stream);
    bytes = fread(output, 1u, sizeof(output) - 1u, stream);
    assert(!ferror(stream));
    assert(bytes > 0u);
    assert(strstr(output, "VNT-A0107") != NULL);
    assert(strstr(output, "How to fix it") != NULL);
    fclose(stream);

    puts("VentuSample audio diagnostics tests passed.");
    return 0;
}
