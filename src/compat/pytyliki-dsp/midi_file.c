#include "midi_file.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    const unsigned char *data;
    size_t size;
    size_t offset;
} midi_reader;

static int read_u16(const unsigned char *data, size_t size, size_t offset, uint16_t *value) {
    if (offset > size || size - offset < 2u || value == NULL) return -1;
    *value = (uint16_t)(((uint16_t)data[offset] << 8u) | (uint16_t)data[offset + 1u]);
    return 0;
}

static int read_u32(const unsigned char *data, size_t size, size_t offset, uint32_t *value) {
    if (offset > size || size - offset < 4u || value == NULL) return -1;
    *value = ((uint32_t)data[offset] << 24u) |
        ((uint32_t)data[offset + 1u] << 16u) |
        ((uint32_t)data[offset + 2u] << 8u) |
        (uint32_t)data[offset + 3u];
    return 0;
}

static int read_vlq(midi_reader *reader, uint32_t *value) {
    uint32_t result = 0u;

    if (reader == NULL || value == NULL) return -1;
    for (unsigned index = 0u; index < 4u; ++index) {
        unsigned char byte;
        if (reader->offset >= reader->size) return -1;
        byte = reader->data[reader->offset++];
        result = (result << 7u) | (uint32_t)(byte & 0x7Fu);
        if ((byte & 0x80u) == 0u) {
            *value = result;
            return 0;
        }
    }
    return -1;
}

static int append_event(
    ventu_midi_event *events,
    size_t *count,
    ventu_midi_event event
) {
    if (*count >= (size_t)VENTU_MIDI_MAX_EVENTS) return -1;
    events[*count] = event;
    *count += 1u;
    return 0;
}

static int parse_track(
    const unsigned char *data,
    size_t size,
    uint32_t track_index,
    ventu_midi_event *events,
    size_t *event_count,
    ventu_midi_summary *summary
) {
    midi_reader reader = {data, size, 0u};
    uint64_t tick = 0u;
    unsigned char running_status = 0u;
    int ended = 0;

    while (reader.offset < reader.size) {
        uint32_t delta;
        unsigned char status;
        unsigned char first_data = 0u;
        unsigned char second_data = 0u;
        unsigned char kind;
        unsigned char channel;

        if (read_vlq(&reader, &delta) != 0 ||
            tick > UINT64_MAX - (uint64_t)delta) {
            return -1;
        }
        tick += (uint64_t)delta;
        if (reader.offset >= reader.size) return -1;

        status = reader.data[reader.offset];
        if (status < 0x80u) {
            if (running_status < 0x80u || running_status >= 0xF0u) return -1;
            status = running_status;
        } else {
            reader.offset += 1u;
            if (status < 0xF0u) running_status = status;
            else running_status = 0u;
        }

        if (status == 0xFFu) {
            unsigned char meta_type;
            uint32_t length;
            if (reader.offset >= reader.size) return -1;
            meta_type = reader.data[reader.offset++];
            if (read_vlq(&reader, &length) != 0 ||
                length > reader.size - reader.offset) return -1;

            if (meta_type == 0x51u) {
                ventu_midi_event event = {0};
                if (length != 3u) return -1;
                event.tick = tick;
                event.track = track_index;
                event.type = VENTU_MIDI_TEMPO;
                event.value = ((uint32_t)reader.data[reader.offset] << 16u) |
                    ((uint32_t)reader.data[reader.offset + 1u] << 8u) |
                    (uint32_t)reader.data[reader.offset + 2u];
                if (event.value == 0u || append_event(events, event_count, event) != 0) return -1;
                summary->tempo_count += 1u;
            } else if (meta_type == 0x58u) {
                ventu_midi_event event = {0};
                unsigned char denominator;
                if (length != 4u) return -1;
                denominator = reader.data[reader.offset + 1u];
                if (denominator > 7u) return -1;
                event.tick = tick;
                event.track = track_index;
                event.type = VENTU_MIDI_TIME_SIGNATURE;
                event.data1 = reader.data[reader.offset];
                event.data2 = (uint8_t)(1u << denominator);
                event.value = reader.data[reader.offset + 2u];
                if (event.data1 == 0u || append_event(events, event_count, event) != 0) return -1;
                summary->time_signature_count += 1u;
            } else if (meta_type == 0x2Fu) {
                if (length != 0u) return -1;
                ended = 1;
                reader.offset += length;
                break;
            }
            reader.offset += length;
            continue;
        }

        if (status == 0xF0u || status == 0xF7u) {
            uint32_t length;
            if (read_vlq(&reader, &length) != 0 || length > reader.size - reader.offset) return -1;
            reader.offset += length;
            continue;
        }
        if (status >= 0xF0u) return -1;

        kind = (unsigned char)(status & 0xF0u);
        channel = (unsigned char)(status & 0x0Fu);
        if (reader.data[reader.offset] < 0x80u) {
            first_data = reader.data[reader.offset++];
        } else if (running_status != 0u) {
            return -1;
        } else {
            return -1;
        }

        if (kind == 0xC0u || kind == 0xD0u) {
            second_data = 0u;
        } else {
            if (reader.offset >= reader.size || reader.data[reader.offset] >= 0x80u) return -1;
            second_data = reader.data[reader.offset++];
        }

        if (kind == 0x80u || kind == 0x90u) {
            ventu_midi_event event = {0};
            event.tick = tick;
            event.track = track_index;
            event.channel = channel;
            event.data1 = first_data;
            event.data2 = second_data;
            event.type = (uint8_t)((kind == 0x90u && second_data != 0u)
                ? VENTU_MIDI_NOTE_ON : VENTU_MIDI_NOTE_OFF);
            if (append_event(events, event_count, event) != 0) return -1;
            if (event.type == VENTU_MIDI_NOTE_ON) summary->note_on_count += 1u;
            else summary->note_off_count += 1u;
        } else if (kind == 0xC0u) {
            ventu_midi_event event = {0};
            event.tick = tick;
            event.track = track_index;
            event.channel = channel;
            event.type = VENTU_MIDI_PROGRAM_CHANGE;
            event.data1 = first_data;
            if (append_event(events, event_count, event) != 0) return -1;
        }
    }

    return (ended || reader.offset == reader.size) ? 0 : -1;
}

int ventu_midi_parse(
    const unsigned char *data,
    size_t size,
    ventu_midi_event *events,
    size_t event_capacity,
    ventu_midi_summary *summary
) {
    uint32_t header_length;
    uint16_t format;
    uint16_t track_count;
    uint16_t division;
    size_t offset;
    size_t event_count = 0u;
    ventu_midi_event *temporary = NULL;
    ventu_midi_summary result = {0};

    if (data == NULL || events == NULL || summary == NULL ||
        size < 14u || size > (size_t)VENTU_MIDI_MAX_BYTES ||
        memcmp(data, "MThd", 4u) != 0 ||
        read_u32(data, size, 4u, &header_length) != 0 ||
        header_length < 6u || header_length > size - 8u ||
        read_u16(data, size, 8u, &format) != 0 ||
        read_u16(data, size, 10u, &track_count) != 0 ||
        read_u16(data, size, 12u, &division) != 0 ||
        format > 1u || track_count == 0u ||
        track_count > VENTU_MIDI_MAX_TRACKS ||
        (format == 0u && track_count != 1u) ||
        division == 0u || (division & 0x8000u) != 0u) {
        return -1;
    }

    temporary = (ventu_midi_event *)calloc(
        (size_t)VENTU_MIDI_MAX_EVENTS, sizeof(*temporary)
    );
    if (temporary == NULL) return -3;

    result.format = format;
    result.track_count = track_count;
    result.ticks_per_quarter = division;
    offset = 8u + (size_t)header_length;

    for (uint32_t track = 0u; track < track_count; ++track) {
        uint32_t chunk_length;
        if (offset > size || size - offset < 8u ||
            memcmp(data + offset, "MTrk", 4u) != 0 ||
            read_u32(data, size, offset + 4u, &chunk_length) != 0 ||
            (size_t)chunk_length > size - (offset + 8u)) {
            free(temporary);
            return -1;
        }
        if (parse_track(data + offset + 8u, (size_t)chunk_length,
                        track, temporary, &event_count, &result) != 0) {
            free(temporary);
            return -1;
        }
        offset += 8u + (size_t)chunk_length;
    }

    if (offset != size) {
        free(temporary);
        return -1;
    }
    if (event_capacity < event_count) {
        free(temporary);
        return -2;
    }

    memcpy(events, temporary, event_count * sizeof(*events));
    result.event_count = event_count;
    *summary = result;
    free(temporary);
    return 0;
}
