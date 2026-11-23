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


## Band-limited offline sample-rate conversion

`resample.h` also exposes `ventu_audio_resample_sinc`. It uses a 16-tap Blackman-windowed sinc kernel and lowers the cutoff in proportion to the output/input sample-rate ratio when downsampling.

- It preserves the same input, channel, rate, frame and output-buffer bounds as the linear path.
- It operates independently on each channel and normalizes the truncated kernel at the signal edges.
- It rejects non-finite input, invalid output state and non-finite or non-representable results.
- The caller receives a completed output buffer only after the full conversion succeeds.
- It is intended for deterministic offline rendering; it is not a real-time audio callback or a claim of equivalence to a dedicated production resampling library.

Keep the linear function for quick previews. Use the windowed-sinc function when reducing sample rate and alias suppression matters. The regression test compares an above-Nyquist tone against the linear preview path and checks that the passband and stereo channels are preserved.

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

The sources are part of \`ventusample_pytyliki_dsp\`. CTest targets \`ventusample-resample\` and \`ventusample-midi-file\` cover linear and windowed-sinc conversion, anti-alias behavior, channel preservation, boundaries, metadata, notes, capacity failures and malformed data.

\`\`\`sh
cmake -S . -B build -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
\`\`\`
