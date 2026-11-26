# VentuSample Meter Lab — integrated WAV analysis

VentuSample Meter Lab is the measurement and comparison layer for the native sample workstation. Its pipeline stays local and streaming:

1. Decode supported integer PCM blocks without loading the whole recording into memory.
2. Accumulate per-channel sample peak, RMS energy, DC offset, crest factor, near-full-scale counts and the frame at which each peak occurred.
3. For stereo files, estimate left/right correlation, Mid/Side energy ratio and the peak of a simple mono fold-down.
4. Produce a versioned measurement report or compare two renders by channel-level peak, RMS and DC deltas.

Examples:

    python tools/ventu_wav_inspect.py sample.wav
    python tools/ventu_wav_inspect.py sample.wav --json --output reports/meter.json
    python tools/ventu_wav_inspect.py before.wav --compare after.wav --output reports/ab-comparison.json

The analysis report schema is ventusample.meter-report.v2. A/B reports use ventusample.meter-comparison.v1. PCM integer widths of 8, 16, 24 and 32 bit are supported, with one to eight channels and sample rates up to 768 kHz. The reader processes a bounded block at a time and records a SHA-256 fingerprint for each input.

Peak is sample-peak, not oversampled true peak. RMS is not BS.1770 integrated loudness or LUFS. The A/B comparison does not time-align edits, loudness-match renders or produce a perceptual quality score. Stereo correlation and Mid/Side figures describe the first two channels of a multichannel file. No audio is modified, normalized, played or uploaded.

This is an original VentuSample analysis subsystem, not a clone of an audio editor or mastering product. It contains no third-party source, artwork, interface layout or copied trademarks. Copyright (c) 2026 Ivan-VHS. The repository Apache-2.0 LICENSE remains authoritative.

Test:

    python -m unittest discover -s tests -p "test_ventu_wav_inspect.py" -v
