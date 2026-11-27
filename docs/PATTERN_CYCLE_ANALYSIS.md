# Repeating pattern-cycle analysis

The optional pattern-cycle helper compares scheduled musical event offsets in repeated fixed-length sections. It is useful for checking whether a loop repeats a motif or introduces a deliberate variation.

## Contract

Inputs are a non-decreasing list of event frame positions, the total frame count and a positive cycle length in frames. The total length must contain whole cycles. The analyzer rejects out-of-range events, unsorted positions, more than 1,000,000 events, or more than 4,096 cycles.

The first cycle is the reference motif. Every following cycle is compared by its event offsets relative to that cycle's start. A cycle matches only when its ordered offsets (including repeated positions) are identical.

The reported percentage is:

repeatAccuracyPercent = 100 * matchingCycles / cycleCount

A structurally valid input with fewer than two cycles or no events is marked as not analyzed rather than being assigned a misleading score. The analysis is deterministic and reports only; it does not modify audio, move notes, render a file, open an audio device, or make a real-time callback.

## Build and verification

The helper is part of `ventusample_core` and has a dedicated CTest target. Tests cover exact repeats, one changed onset, a missing event, empty and single-cycle inputs, unsorted or out-of-range positions, incomplete cycles and the work bound.
