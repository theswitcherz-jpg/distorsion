# SPCTRL-D — Asymmetric Distortion / Compressor / Clipper

VST3 audio effect built with JUCE.

## Signal chain

INPUT → ASYMMETRIC DISTORTION → TONE / DC CLEANUP → COMPRESSOR → MAKEUP → HARD CLIPPER → OUTPUT

## Controls

### Distortion
- Drive: 0–36 dB
- Asymmetry: -100% to +100%, with independent positive/negative saturation curves
- Tone: low-pass/high-frequency blend
- Dist Mix: dry/wet

### Compressor
- Threshold: -48 to 0 dB
- Ratio: 1:1 to 20:1
- Attack: 0.1–100 ms
- Release: 10–500 ms
- Makeup: 0–18 dB

### Clipper
- Clip: 0–12 dB of final peak reduction
- Clip Mix: parallel clipping amount
- Output: -24 to +6 dB

The supplied `Assets/logo.png` is used as the SPCTRL-D plugin logo.

## Build

Place a JUCE checkout in `./JUCE` or pass `-DJUCE_DIR=/path/to/JUCE`.

```bash
cmake -B build -S . -DJUCE_DIR=/path/to/JUCE
cmake --build build --config Release
```

The resulting VST3 is produced by JUCE/CMake in the build tree and, with `COPY_PLUGIN_AFTER_BUILD TRUE`, is copied to the normal VST3 location where supported.
