## What changed

Describe the sound-engine, export-format or desktop workflow improvement.

## Verification

- [ ] DSP core tests pass.
- [ ] Preset/manifest/WAV format tests pass when relevant.
- [ ] CMake and CTest pass on a Qt-enabled host for UI changes.
- [ ] Deterministic seed behaviour and output bounds are preserved.

## Audio and privacy review

- [ ] No hidden network access or realtime audio side effects were added.
- [ ] Imported source paths and local machine details are not exported unintentionally.
- [ ] Hashes are described as byte identity, not proof of authorship or rights.
- [ ] No large generated audio assets or private samples were committed.
