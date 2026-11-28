# Read-only audio diagnostics

The C++20 `ventu::analyzeAudio` function provides a signal report without
modifying the supplied `AudioBuffer`. It measures peak, RMS, crest factor,
overall DC offset, clipping-threshold crossings and stereo correlation where
the input has at least two channels.

```cpp
#include "audio_diagnostics.hpp"

const ventu::AudioDiagnostics report = ventu::analyzeAudio(audio);
if (report.valid) {
    // report.peak, report.rms, report.dcOffset,
    // report.clippedSamples and report.stereoCorrelation
}
```

The default clipping threshold is 0.999 in normalized linear PCM units. A
caller may provide another finite threshold in the interval `(0, 1]`. The
function rejects unsupported sample rates, channel counts, incomplete frames
and non-finite samples. A mono buffer does not claim a stereo-correlation
measurement.
