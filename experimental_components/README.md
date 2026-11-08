# Local classic-ESP32 experiment — NOT an upstream-supported target

`esp_audio_stack/` is vendored from
[n-IA-hane/esphome-audio-stack](https://github.com/n-IA-hane/esphome-audio-stack/tree/31b576975cae32379df8a025826e03b58bdd1069/esphome/components/esp_audio_stack),
commit **31b576975cae32379df8a025826e03b58bdd1069** (the Radio's known backend).
Its original MIT license is retained in `UPSTREAM_LICENSE`.

Local port change: `__init__.py` also admits the real `ESP32` variant with two
standard I²S ports and pins `esp_audio_effects` to the resolved version 1.4.4.
No TDM or new APLL support is asserted. This does NOT mean
the upstream project supports classic ESP32. The C++ bus code still needs real
Luxe timing, DMA, clock, and endurance validation.

`audio_core_ring_buffer_caps.h` is adapted to ESPHome 2026.10's `handle_()`
accessor and base-owned static-handle lifetime. The base destructor alone frees
the backing store; installation rejects an already allocated buffer.

`luxe_aec/` is a new, separate adapter using Espressif ESP-SR **2.6.0**, ESP-DSP
**1.8.0**, and the classic chip's `aec_create` / `aec_process` / `aec_destroy`
API, rather than the Radio's `afe_aec_*` API. Only mono 16 kHz `SR_LOW_COST` is
offered. All three PCM buffers passed into Espressif are aligned internal-RAM
allocations. No SR wake-word model partition is used: wake-word inference is
still ESPHome's Micro Wake Word.

These files are retained from separate, paused AEC and Stop experiments.
The published `luxe.yaml` imports neither component. The experimental YAMLs
and their test notes are not included in this publication directory.
