# VentuSample Reaper Neural Audio Contract v1

**Adapter:** integrations/reaper/neural_audio_contract.rpr  
**Status:** offline profile validation; not a neural DSP engine

## Audio profile

The ten-field profile contains:

1. ventusample.neural-audio-profile.v1;
2. sample rate: 48,000 or 96,000 Hz;
3. channel count: mono or stereo;
4. sample format: s24le or f32le;
5. processing block size: 64–8,192 frames, divisible by 16;
6. deterministic u32 seed;
7. execution context: offline_worker;
8. mode: inference or offline_batch;
9. external audio upload flag, required false;
10. real-time audio callback flag, required false.

A valid profile returns ALLOW_OFFLINE with deterministic worker-thread execution, no callback and no external upload.

## Audio-thread and privacy boundary

Neural work must remain outside the audio callback. This contract does not claim real-time inference, does not move DSP to the GUI thread, does not upload audio, and does not change the existing offline DSP result. A production worker must separately own buffering, cancellation, deadlines, memory reservation and result handoff.

The adapter does not add a plugin host or a neural audio engine.

## Verification

The Reaper native integration workflow checks and executes the fixture and requires VENTUSAMPLE_REAPER_NEURAL_AUDIO:PASS. Invalid sample rate, unaligned blocks, callback execution and external upload are rejected.
