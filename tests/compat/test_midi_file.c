#include "midi_file.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <string.h>

int main(void) {
    static const unsigned char midi[] = {
        'M','T','h','d', 0,0,0,6, 0,1, 0,2, 0,96,
        'M','T','r','k', 0,0,0,19,
        0,0xFF,0x51,3,0x07,0xA1,0x20,
        0,0xFF,0x58,4,4,2,24,8,
        0,0xFF,0x2F,0,
        'M','T','r','k', 0,0,0,12,
        0,0x90,60,100,
        0x60,0x80,60,0,
        0,0xFF,0x2F,0
    };
    ventu_midi_event events[8];
    ventu_midi_summary summary;

    memset(&summary, 0, sizeof(summary));
    assert(ventu_midi_parse(midi, sizeof(midi), events, 8u, &summary) == 0);
    assert(summary.format == 1u);
    assert(summary.track_count == 2u);
    assert(summary.ticks_per_quarter == 96u);
    assert(summary.event_count == 4u);
    assert(summary.tempo_count == 1u);
    assert(summary.time_signature_count == 1u);
    assert(summary.note_on_count == 1u);
    assert(summary.note_off_count == 1u);
    assert(events[0].type == VENTU_MIDI_TEMPO);
    assert(events[0].value == 500000u);
    assert(events[1].type == VENTU_MIDI_TIME_SIGNATURE);
    assert(events[1].data1 == 4u && events[1].data2 == 4u);
    assert(events[2].type == VENTU_MIDI_NOTE_ON && events[2].tick == 0u);
    assert(events[3].type == VENTU_MIDI_NOTE_OFF && events[3].tick == 96u);

    assert(ventu_midi_parse(midi, sizeof(midi), events, 3u, &summary) == -2);
    assert(ventu_midi_parse(midi, sizeof(midi) - 2u, events, 8u, &summary) == -1);
    assert(ventu_midi_parse(NULL, 0u, events, 8u, &summary) == -1);
    return 0;
}
