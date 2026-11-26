#ifndef VENTUSAMPLE_MIDI_FILE_H
#define VENTUSAMPLE_MIDI_FILE_H

#include <stddef.h>
#include <stdint.h>

#define VENTU_MIDI_MAX_TRACKS 256u
#define VENTU_MIDI_MAX_EVENTS 1000000u
#define VENTU_MIDI_MAX_BYTES (32u * 1024u * 1024u)

typedef enum {
    VENTU_MIDI_NOTE_OFF = 1,
    VENTU_MIDI_NOTE_ON = 2,
    VENTU_MIDI_PROGRAM_CHANGE = 3,
    VENTU_MIDI_TEMPO = 4,
    VENTU_MIDI_TIME_SIGNATURE = 5
} ventu_midi_event_type;

typedef struct {
    uint64_t tick;
    uint32_t track;
    uint8_t channel;
    uint8_t type;
    uint8_t data1;
    uint8_t data2;
    uint32_t value;
} ventu_midi_event;

typedef struct {
    uint16_t format;
    uint16_t track_count;
    uint16_t ticks_per_quarter;
    size_t event_count;
    size_t note_on_count;
    size_t note_off_count;
    size_t tempo_count;
    size_t time_signature_count;
} ventu_midi_summary;

/*
 * Parse Standard MIDI File format 0 or 1.
 * The caller owns the event buffer. On error, neither events nor summary
 * are modified. SMPTE time division and format 2 are intentionally rejected.
 */
int ventu_midi_parse(
    const unsigned char *data,
    size_t size,
    ventu_midi_event *events,
    size_t event_capacity,
    ventu_midi_summary *summary
);

#endif
