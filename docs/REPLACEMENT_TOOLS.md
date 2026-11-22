# Original replacement tools

The new tools/ventu_wav_inspect.py utility is an independently written, read-only PCM WAV analyzer for the VentuSample audio workflow. It streams audio blocks and reports per-channel peak, RMS, DC offset, crest factor and near-full-scale sample counts for supported uncompressed PCM files.

Run:

    python tools/ventu_wav_inspect.py sample.wav --json

It supports PCM sample widths of 8, 16, 24 and 32 bit, mono or multichannel WAV up to eight channels, and bounded file sizes. It does not alter, normalize, play or upload audio. It does not claim BS.1770 integrated loudness, oversampled true peak, restoration or mastering compliance; those require separate algorithms and validation.

This is not a clone of RX, WaveLab, SPAN, Youlean Loudness Meter, FFmpeg or another named product. It implements original measurements without third-party source code, artwork, UI layout or trademarks. Product names in the source catalogue are reference points for broad use-cases only.

The new source and tests are original VentuSample contributions, copyright (c) 2026 Ivan-VHS, under Apache License 2.0. The repository LICENSE remains authoritative.

Test:

    python -m unittest discover -s tests -p "test_ventu_wav_inspect.py" -v
