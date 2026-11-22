# MIDI parsing and offline sample-rate conversion

This implementation wave adds two bounded C11 capabilities to the offline processing library.

## Linear sample-rate conversion

\`resample.h\` exposes \`ventu_audio_resample_linear\`. It converts mono or stereo in-memory buffers between 8 kHz and 192 kHz by deterministic linear interpolation.

- Input and output are separate caller-owned buffers.
- The output frame count is rounded from the source duration and target rate.
- The existing 4,800,000-frame bound is enforced before allocation.
- Non-finite samples, unsupported channel counts and invalid rates are rejected.
- The output structure is published only after the complete conversion succeeds.

Linear interpolation is a compact preview-quality method, not a replacement for a band-limited production resampler such as SoXR or libsamplerate. It does not perform anti-alias filtering.

## Standard MIDI File inspection

\`midi_file.h\` parses Standard MIDI File formats 0 and 1 from caller-supplied bytes. It reports note-on/note-off events, program changes, tempo events, time signatures, track count and ticks per quarter note.

The parser:
- accepts at most 32 MiB, 256 tracks and 1,000,000 extracted events;
- checks every chunk, variable-length quantity, event payload and tick accumulation;
- supports channel running status, tempo and time-signature metadata;
- rejects SMPTE time division, format 2 and malformed/truncated chunks;
- never opens a MIDI device, schedules playback or modifies an input file.

The parser is an importer, not a player or sequencer. The event buffer is supplied by the caller. If it is too small, parsing returns \`-2\` without publishing a partial event list or summary.

## Build and test

The sources are part of \`ventusample_pytyliki_dsp\`. CTest targets \`ventusample-resample\` and \`ventusample-midi-file\` cover interpolation, boundaries, metadata, notes, capacity failures and malformed data.

\`\`\`sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
\`\`\`
