# VentuSample pack manifest audit

VentuSample owns both the pack format and its optional metadata audit. The audit reads a JSON manifest only. It does not open, decode, render, modify or upload the asset files named in that manifest.

## API

The standalone implementation lives in `tools/ventusample_pack_audit.py`:

- `ventusample_metrics(pack)` validates the complete `ventusample.pack.v1` structure and returns 22 sorted normalized values.
- `compare_ventusample_packs(reference, candidate)` compares two validated manifests, reports field-by-field differences and computes a deterministic SHA-256 report digest.

The comparison uses Python's standard library only. It does not import Reaper, Pytyliki or a native library.

## Validation boundaries

The validator checks exact fields, supported parameter ranges, sample-rate/filter relationships, safe flat filenames, duplicate assets, finite measurement values and the declared per-variation file counts. A manifest is not proof that its referenced files exist, are authentic or are safe to render. The audit does not inspect file bytes.

## Tests

The test suite covers supported profile values, metric normalization, deterministic comparison output, changed parameter/measurement reporting, path traversal rejection, non-finite metric rejection and variation-count boundaries:

```sh
python -m unittest tests.test_ventusample_pack_audit -v
```

The same test is registered with CTest when a Python interpreter is available.
