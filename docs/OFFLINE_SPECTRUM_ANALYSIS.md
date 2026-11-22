# Offline spectrum analysis

The C11 processing library includes a bounded radix-2 FFT analyzer. It is separate from the generator and has no external DSP dependency.

`pl_audio_analyze_spectrum` analyzes the first N frames of one selected channel or a simple channel average. N must be a power of two from 256 through 65,536, and the input must be a supported in-memory audio buffer. The caller supplies storage for N/2+1 one-sided magnitude bins.

A Hann window is applied. Magnitudes are normalized by the window's coherent gain. The report includes bin width, peak frequency/magnitude, magnitude-weighted spectral centroid and the 85% magnitude rolloff frequency. Silence produces zero-valued spectral summaries. Invalid input and non-finite samples fail closed.

The analyzer is offline and buffer-based. It does not open audio devices, schedule real-time callbacks or modify the source audio buffer.

## Catalog links

This adopts the analysis workflow patterns of FFTW/KissFFT, SciPy signal, Essentia, aubio and spectrum analyzers (catalog items 121–170 and 941–989), using an original small C11 implementation rather than bundling external code.
