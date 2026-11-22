#include "midi_export.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <system_error>
#include <utility>
#include <vector>

namespace ventu {
namespace {

constexpr std::uint16_t kPpqn = 480;
constexpr std::size_t kMaximumNotes = 100000u;

struct TrackEvent {
    std::uint64_t tick{0};
    unsigned order{0};
    std::vector<std::uint8_t> bytes;
};

struct Track {
    std::vector<TrackEvent> events;
};

std::uint64_t frameToTick(std::size_t frame, int sampleRate, int bpm) {
    const long double ticks = static_cast<long double>(frame)
        * static_cast<long double>(bpm) * static_cast<long double>(kPpqn)
        / (60.0L * static_cast<long double>(sampleRate));
    return static_cast<std::uint64_t>(ticks + 0.5L);
}

void addMeta(Track& track, std::uint64_t tick, std::initializer_list<std::uint8_t> data) {
    track.events.push_back({tick, 0u, std::vector<std::uint8_t>(data)});
}

void addNote(Track& track, std::uint64_t start, std::uint64_t end,
             std::uint8_t channel, int note, int velocity) {
    const auto pitch = static_cast<std::uint8_t>(note);
    const auto strength = static_cast<std::uint8_t>(velocity);
    track.events.push_back({start, 1u,
        {static_cast<std::uint8_t>(0x90u | channel), pitch, strength}});
    track.events.push_back({end, 0u,
        {static_cast<std::uint8_t>(0x80u | channel), pitch, 0u}});
}

void appendVariableLength(std::vector<std::uint8_t>& output, std::uint64_t value) {
    std::array<std::uint8_t, 10> encoded{};
    std::size_t used = 0u;
    encoded[used++] = static_cast<std::uint8_t>(value & 0x7Fu);
    value >>= 7u;
    while (value != 0u && used < encoded.size()) {
        encoded[used++] = static_cast<std::uint8_t>((value & 0x7Fu) | 0x80u);
        value >>= 7u;
    }
    while (used > 0u) output.push_back(encoded[--used]);
}

std::vector<std::uint8_t> encodeTrack(Track& track) {
    std::stable_sort(track.events.begin(), track.events.end(),
        [](const TrackEvent& left, const TrackEvent& right) {
            if (left.tick != right.tick) return left.tick < right.tick;
            return left.order < right.order;
        });

    std::vector<std::uint8_t> encoded;
    std::uint64_t previousTick = 0u;
    for (const auto& event : track.events) {
        const std::uint64_t delta = event.tick - previousTick;
        appendVariableLength(encoded, delta);
        encoded.insert(encoded.end(), event.bytes.begin(), event.bytes.end());
        previousTick = event.tick;
    }
    appendVariableLength(encoded, 0u);
    encoded.insert(encoded.end(), {0xFFu, 0x2Fu, 0x00u});
    return encoded;
}

void append16(std::vector<std::uint8_t>& output, std::uint16_t value) {
    output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>(value & 0xFFu));
}

void append32(std::vector<std::uint8_t>& output, std::uint32_t value) {
    output.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>(value & 0xFFu));
}

void addTrackName(Track& track, const char* name) {
    std::vector<std::uint8_t> event{0xFFu, 0x03u};
    std::size_t length = 0u;
    while (name[length] != '\0') {
        event.push_back(static_cast<std::uint8_t>(name[length]));
        ++length;
    }
    event.insert(event.begin() + 2, static_cast<std::uint8_t>(length));
    track.events.push_back({0u, 0u, std::move(event)});
}

bool addRendererNotes(const Parameters& parameters, const AudioBuffer& audio,
                      std::array<Track, 3>& tracks, std::string& error) {
    if (audio.channels != 2 || audio.sampleRate != parameters.sampleRate ||
        audio.samples.empty() || audio.samples.size() % 2u != 0u) {
        error = "MIDI export requires a valid stereo render from the current parameters.";
        return false;
    }
    if (audio.midiNotes.empty() || audio.midiNotes.size() > kMaximumNotes) {
        error = "The render contains no MIDI notes or exceeds the note-event limit.";
        return false;
    }

    const std::size_t frameCount = audio.samples.size() / 2u;
    for (const auto& note : audio.midiNotes) {
        if (note.startFrame >= frameCount || note.durationFrames == 0u ||
            note.note < 0 || note.note > 127 || note.velocity < 1 || note.velocity > 127) {
            error = "The render contains a MIDI note outside the supported bounds.";
            return false;
        }
        std::size_t endFrame = frameCount;
        if (note.durationFrames <= frameCount - note.startFrame)
            endFrame = note.startFrame + note.durationFrames;
        if (endFrame <= note.startFrame) {
            error = "The render contains a MIDI note with an invalid duration.";
            return false;
        }

        std::size_t trackIndex = 0u;
        std::uint8_t channel = 9u;
        switch (note.part) {
        case MidiPart::Drums:
            trackIndex = 0u;
            channel = 9u;
            break;
        case MidiPart::Bass:
            trackIndex = 1u;
            channel = 0u;
            break;
        case MidiPart::Melody:
            trackIndex = 2u;
            channel = 1u;
            break;
        default:
            error = "The render contains an unknown MIDI part.";
            return false;
        }

        const auto start = frameToTick(note.startFrame, audio.sampleRate, parameters.bpm);
        const auto end = std::max(start + 1u,
            frameToTick(endFrame, audio.sampleRate, parameters.bpm));
        addNote(tracks[trackIndex], start, end, channel, note.note, note.velocity);
    }
    return true;
}

void addOneShotArrangement(const Parameters& parameters, std::array<Track, 3>& tracks) {
    // Eight original kit slots over one bar. This is a MIDI arrangement only;
    // no third-party sample or recording is included in the file.
    struct Slot {
        MidiPart part;
        int note;
        int velocity;
    };
    const std::array<Slot, 8> slots{{
        {MidiPart::Drums, 36, 108},
        {MidiPart::Drums, 38, 96},
        {MidiPart::Drums, 42, 76},
        {MidiPart::Drums, 39, 90},
        {MidiPart::Drums, 37, 82},
        {MidiPart::Drums, 76, 78},
        {MidiPart::Bass, 36 + parameters.rootNote, 88},
        {MidiPart::Melody, 60 + parameters.rootNote, 80},
    }};
    for (std::size_t index = 0u; index < slots.size(); ++index) {
        const auto& slot = slots[index];
        const std::size_t trackIndex = slot.part == MidiPart::Drums
            ? 0u : (slot.part == MidiPart::Bass ? 1u : 2u);
        const std::uint8_t channel = slot.part == MidiPart::Drums
            ? 9u : (slot.part == MidiPart::Bass ? 0u : 1u);
        const std::uint64_t start = static_cast<std::uint64_t>(index) * 240u;
        addNote(tracks[trackIndex], start, start + 120u, channel, slot.note, slot.velocity);
    }
}

bool writeFile(const std::filesystem::path& destination,
               const std::vector<std::uint8_t>& bytes, std::string& error) {
    if (destination.empty() || destination.filename().empty()) {
        error = "Choose a destination filename for the MIDI export.";
        return false;
    }
    std::error_code filesystemError;
    if (std::filesystem::exists(destination, filesystemError) || filesystemError) {
        error = "The MIDI destination already exists or cannot be checked; choose another filename.";
        return false;
    }

    auto temporary = destination;
    temporary += ".tmp";
    if (std::filesystem::exists(temporary, filesystemError) || filesystemError) {
        error = "The temporary MIDI export path already exists or cannot be checked.";
        return false;
    }

    {
        std::ofstream file(temporary, std::ios::binary | std::ios::out);
        if (!file) {
            error = "Could not create the temporary MIDI file: " + temporary.string();
            return false;
        }
        file.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
        file.flush();
        if (!file.good()) {
            file.close();
            std::filesystem::remove(temporary, filesystemError);
            error = "An error occurred while writing the MIDI file.";
            return false;
        }
    }

    if (std::filesystem::exists(destination, filesystemError) || filesystemError) {
        std::filesystem::remove(temporary, filesystemError);
        error = "The MIDI destination appeared during export; no file was overwritten.";
        return false;
    }
    std::filesystem::rename(temporary, destination, filesystemError);
    if (filesystemError) {
        std::filesystem::remove(temporary, filesystemError);
        error = "Could not finalize the MIDI export.";
        return false;
    }
    return true;
}

} // namespace

bool writeMidiFile(const std::filesystem::path& path, const Parameters& parameters,
                   const AudioBuffer& audio, std::string& error) {
    error.clear();
    if (const auto issue = validate(parameters); !issue.empty()) {
        error = issue;
        return false;
    }

    std::array<Track, 3> parts;
    addTrackName(parts[0], "Drums");
    addTrackName(parts[1], "Bass");
    addTrackName(parts[2], "Melody");

    if (parameters.target == Target::OneShotKit) {
        addOneShotArrangement(parameters, parts);
    } else if (!addRendererNotes(parameters, audio, parts, error)) {
        return false;
    }

    Track conductor;
    addTrackName(conductor, "Conductor");
    const std::uint32_t microsecondsPerQuarter =
        60000000u / static_cast<std::uint32_t>(parameters.bpm);
    addMeta(conductor, 0u, {
        0xFFu, 0x51u, 0x03u,
        static_cast<std::uint8_t>((microsecondsPerQuarter >> 16u) & 0xFFu),
        static_cast<std::uint8_t>((microsecondsPerQuarter >> 8u) & 0xFFu),
        static_cast<std::uint8_t>(microsecondsPerQuarter & 0xFFu)
    });
    addMeta(conductor, 0u, {0xFFu, 0x58u, 0x04u, 0x04u, 0x02u, 0x18u, 0x08u});

    const std::array<std::vector<std::uint8_t>, 4> encodedTracks{{
        encodeTrack(conductor),
        encodeTrack(parts[0]),
        encodeTrack(parts[1]),
        encodeTrack(parts[2]),
    }};
    std::vector<std::uint8_t> file;
    file.insert(file.end(), {'M', 'T', 'h', 'd'});
    append32(file, 6u);
    append16(file, 1u);
    append16(file, static_cast<std::uint16_t>(encodedTracks.size()));
    append16(file, kPpqn);

    for (const auto& track : encodedTracks) {
        if (track.size() > std::numeric_limits<std::uint32_t>::max()) {
            error = "A MIDI track exceeded the Standard MIDI File size limit.";
            return false;
        }
        file.insert(file.end(), {'M', 'T', 'r', 'k'});
        append32(file, static_cast<std::uint32_t>(track.size()));
        file.insert(file.end(), track.begin(), track.end());
    }
    return writeFile(path, file, error);
}

} // namespace ventu
