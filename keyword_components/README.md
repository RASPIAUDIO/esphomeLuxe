# Local keyword diagnostic snapshot

`micro_wake_word/` is copied from the installed ESPHome **2026.10.1** package
on 2026-11-01 for the separate, paused Stop experiment. The published
`luxe.yaml` does not import it and uses ESPHome's native Micro Wake Word.
The experimental YAML is not included in this publication directory.

Initial modifications are limited to `micro_wake_word.h/.cpp`: atomic counters
for input/consumed samples, generated feature count, frontend/inference wall
times and rejected input chunks. A report is emitted every ten seconds from
the main loop or by Audio Diagnostics. The inference/audio tasks do not log
these reports. The model thresholds, memory placement, task priority, buffer
duration and recognition behavior are unchanged in `stop2-diag`.

These are wall-clock timings, including preemption; they are not pure CPU
cycle measurements. Compare stable windows after model warm-up. The default
10 ms feature step needs about 100 features/s and 16,000 microphone samples/s.
An overflow count records rejected chunks, not a count of spoken words.

Original SHA-256 values for the modified source files:

- `micro_wake_word.h`: `d8f35206bb56722ce69809653f59998290a9794f9b404fbfe2f4a9a857a83409`
- `micro_wake_word.cpp`: `d042c89920d2877dec565349063a5c8e86476ed22abebea21f5fb12ac22c4893`

`stop3` changes only the hot tensor-arena allocation in `streaming_model.cpp`:
on classic ESP32 it prefers internal memory when the contiguous block fits and
leaves at least 64 KiB free at the allocation check. Otherwise it retains PSRAM;
the internal preference also falls back to PSRAM if allocation cannot succeed.
The chosen memory region and free internal heap are logged. Other model memory,
thresholds, task priority and ring-buffer size are unchanged for this comparison.

Original `streaming_model.cpp` SHA-256:
`aa2dc589f1f97fb02f8340e74793f88e3e152380643bb61a9e056a4d8b9ae915`.
The other five files remain copied verbatim from the installed package snapshot.

Hardware showed that the initial 64 KiB guard retained PSRAM: the actual hot
arenas are 28,752 bytes (Nabu) and 16,416 bytes (Stop), with about 78 KiB free
before Stop's allocation. `stop3b` uses a 48 KiB reserve to exercise internal
placement while leaving headroom for the already initialized audio/network path.

Internal placement was confirmed in stop3b but did not sustain 100 features/s
during music. `stop4` retains it and uses `KeywordInferenceTask` to create the
classic-ESP32 inference task directly on core 0, while the audio stack stays
on core 1. Priority, stack size, ring buffer and thresholds stay unchanged.
The helper follows the installed `StaticTask::create` allocation contract and
inherits its existing asynchronous destruction/deallocation. Other targets
fall back to the ordinary StaticTask creator. Verify core, throughput, network,
audio continuity and repeated teardown/rearming on hardware.

Stop5-pcm kept core 0 but switched Sendspin to PCM; the additional network load
coincided with very low inference throughput on that core. Stop6-pcm changes
only the helper's pinned core to 1 for a matched PCM comparison. The earlier
Stop4/core-0 result describes FLAC, not this different transport workload.
