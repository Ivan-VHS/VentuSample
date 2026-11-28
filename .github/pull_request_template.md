This PR explains one focused improvement to VentuSample's audio, DSP, plugin or desktop workflow.

## What changed
Describe what a user or downstream module can now do. Name the affected processing path, format or interface.

## Why
What limitation does this change address?

## Audio and implementation details
- [ ] Buffer, channel, sample-rate and format assumptions are explicit.
- [ ] Failure paths preserve the documented signal and state guarantees.
- [ ] Plugin and preset lifecycle behavior is safe under the supported call patterns.
- [ ] Offline processing is not described as real-time playback.
- [ ] Any performance claim is backed by a repeatable benchmark on named conditions.

## Verification
List exact CMake, CTest, format, DSP and UI checks that were run. Include relevant edge cases and mark pending work honestly.

## Compatibility and privacy
Explain affected project formats, APIs, sample/preset handling and migration behavior. Do not add private samples, machine-specific paths or hidden network behavior.

## Dependencies
Link related DSP, Reaper integration and cross-repository PRs. State merge order where work is stacked.
