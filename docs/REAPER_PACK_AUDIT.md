# Reaper pack audit

VentuSample exports WAV assets and a versioned manifest. Reaper consumes supported pack-v1 JSON metadata as an optional, separate application.

| Operation | Result |
|---|---|
| `ventusample_metrics(pack)` | validate v1 manifest; 22 normalized descriptors |
| `ventusample_compare(reference,candidate[,library])` | descriptor deltas, Euclidean distance, SHA-256 report |
| Pytyliki cross-check | optional explicit trusted C ABI path |

The audit reads JSON only. It does not open WAV files, compare waveforms, listen to samples or score musical quality. Descriptor distance measures metadata/measurement difference, not quality. Pack v2 tonal export remains a separate schema contract.
