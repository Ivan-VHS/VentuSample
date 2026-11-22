# CLife — sample workstation savepoints

The shared OCTA-8 typed-value contract is documented in [OCTA8.md](OCTA8.md); its native C++20 adapter is intentionally kept outside the audio callback.

CLife v1 adds bounded savepoints for VentuSample source trees, project manifests, presets and exported sample-pack assets.

## Software mapping

| Simulation concept | VentuSample implementation |
|---|---|
| Cryo fault | interrupted export, missing file, inconsistent manifest or checksum mismatch |
| Snapshot | compressed archive with per-file SHA-256 records |
| Integrity / coherence | verified source and output-file fingerprints |
| Stasis hold | refuse recovery when the archive is invalid or destination already exists |
| Awakening | restore into a separate working directory for inspection |

CLife does not run in the audio callback, alter the active renderer, hot-swap native code or claim to snapshot a live audio buffer. Deterministic audio generation remains governed by the generator settings and seed recorded by VentuSample's existing manifests.

## Commands

Run from the repository root with Python 3.10+:

```sh
python tools/clife.py snapshot .
python tools/clife.py verify .clife/snapshots/<snapshot>.clife
python tools/clife.py restore .clife/snapshots/<snapshot>.clife --to ../ventusample-recovered
python -m unittest discover -s tests -p "test_clife.py" -v
```

The default archive location is .clife/snapshots/. Build, distribution, version-control, cache, dependency and previous CLife output directories are excluded.

## Safety and limits

- File fingerprints use SHA-256 and are verified before a restore is committed.
- Restore writes only to a new directory. Existing directories are never overwritten or merged.
- Maximums: 20,000 files, 128 MiB per file and 1 GiB total uncompressed data.
- Audio and project files are copied byte-for-byte; they are not rendered, normalized, decoded or modified.
- A restore is a filesystem copy, not a real-time session recovery or VST/AU/CLAP plugin loader.
- No 666,666,666-character pseudocode artifact is generated; the archive contains real project files and a compact manifest.
