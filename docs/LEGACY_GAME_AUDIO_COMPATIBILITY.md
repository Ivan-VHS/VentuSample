# Legacy game audio compatibility

**Project author/developer:** Ivan-VHS

VentuSample adds a format planner for the audio requirements found in Windows XP-era games: DirectSound, WinMM, OpenAL and optional EAX, with common 8/16/24/32-bit PCM widths, mono/stereo/surround layouts and legacy sample rates.

```sh
python tools/legacy_game_audio.py game-audio-request.json --host host-audio-capabilities.json
```

The report selects the nearest declared host sample rate and indicates whether resampling, float-format conversion or downmixing is required before data enters the existing float-buffer DSP path. EAX is reported as available only when the host capability document explicitly says so.

This is a conversion plan, not an audio-device backend. It does not open a device, play audio, implement DirectSound/WinMM/OpenAL APIs or emulate EAX. The existing DSP/resampling code can perform the indicated conversions after a real host audio bridge supplies the samples and device capabilities.
