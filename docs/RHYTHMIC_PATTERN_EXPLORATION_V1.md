# Rhythmic Pattern Exploration Profile v1

**Status:** proposed offline audio profile; the existing renderer and DSP catalogue are not being reclassified as a child-learning application.  
**Scope:** sound as an optional way to explore repetition, grouping, pulse, duration and change.

## 1. Why this belongs in an audio tool

Early mathematics includes patterns and temporal relationships. VentuSample can provide reproducible audio examples for listening and comparing:

- repeated two-part and three-part patterns;
- a repeated element followed by a contrasting element;
- regular pulse and grouped pulse;
- sound versus silence;
- longer/shorter intervals and faster/slower sequences;
- a pattern that changes one event at a time.

Audio is an additional sensory representation. It does not replace counting with physical objects, spatial play, speech or mathematical explanation.

## 2. Pattern vocabulary

| Pattern | Abstract form | Listening example |
|---|---|---|
| Alternating | ABAB | clap, tap, clap, tap |
| Paired repeat | AABAAB | two short taps, one long tap, repeat |
| Three-part cycle | ABCABC | low, high, rest, repeat |
| Growing group | 1, 2, 3, 4 events | one beat, then two, then three |
| Contrast | A then B | short group followed by a longer group |

The manifest should store event identity and timing explicitly. The same visible pattern may be rendered with different timbres without changing its abstract structure.

## 3. Offline rendering contract

- Generate audio offline through the existing bounded render pipeline.
- Use deterministic seeds so the same declared pattern can be reproduced.
- Record the sample rate, BPM or event intervals, bar/step count, swing and pattern version.
- Preserve the existing WAV validation, frame limits, finite-sample checks and output bounds.
- Do not load external plugins or libraries implicitly.
- Do not access a microphone, record a child's voice, connect to an audio device or send telemetry.
- Keep generated examples as optional local assets; the DAK activity must remain usable without an audio backend.

BPM describes musical pulse, not a universal measure of a child's processing speed or learning level. A pattern can also be specified by relative event intervals where a fixed tempo is unnecessary.

## 4. Suggested interaction modes

1. **Listen:** play a short, bounded pattern in a host-controlled player.
2. **Notice:** ask what repeats or what changed; do not reveal an answer immediately.
3. **Predict:** offer a silent pause in which a caregiver and child can guess the next event.
4. **Change one thing:** alter one sound, interval or rest while preserving the rest of the pattern.
5. **Create:** let the child choose from a few safe, large visual tokens that map to sound events.

VentuSample itself remains an offline renderer. Any interactive listening controls belong to the DAK host or another explicitly specified player.

## 5. Accessibility and listening safety

- Every audio pattern needs a visual or tactile equivalent, such as large event tiles or a simple pulse animation.
- The exercise must not require stereo hearing, pitch discrimination or speech.
- Avoid loudness competitions, sudden high-level transients and mandatory headphones.
- The host controls playback volume and exposes stop/pause immediately. The renderer's sample-peak ceiling is not a hearing-safety guarantee.
- Keep examples short and allow repeated listening, silence and early exit without penalty.

## 6. Validation and tests

Before implementation is advertised:

- validate event count, interval, tempo, duration and total rendered-frame bounds;
- reject non-finite timing values, negative durations and invalid event identifiers;
- prove same-seed deterministic output;
- test exact event ordering and rests at buffer boundaries;
- verify silence remains silence and zero-intensity paths are transparent;
- test that the visual/event representation and rendered pattern share the same schedule;
- confirm that no microphone, network, user profile or runtime plugin loading is introduced.

## 7. Federation boundary

DAK owns the learning objective, language, caregiver controls and activity flow. Reaper owns number and pattern semantics where an exercise makes a formal claim. VentuSample owns deterministic offline sound generation only. Pytyliki may optionally provide bounded numeric checks. OPlague may inspect an imported asset package read-only.

The audio path is optional, local and non-assessing. No score, leaderboard, developmental inference or learner telemetry belongs in this profile.
