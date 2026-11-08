#pragma once
#include <string>

namespace luxe_stop {
enum class Mode { NONE, IDLE_NABU, MUSIC_STOP };

inline Mode wanted_mode(bool ha_ready, bool enabled, bool muted, bool session,
                        bool music, bool local_audio, bool speaker_stopped,
                        bool voice_running, bool music_wake_enabled, bool audio_ready) {
  if (!ha_ready || !enabled || muted || session || local_audio || voice_running || !audio_ready)
    return Mode::NONE;
  if (music)
    return music_wake_enabled ? Mode::MUSIC_STOP : Mode::NONE;
  return speaker_stopped ? Mode::IDLE_NABU : Mode::NONE;
}

inline bool accepts(Mode mode, const std::string &word) {
  return (mode == Mode::IDLE_NABU && word == "Okay Nabu") ||
         (mode == Mode::MUSIC_STOP && word == "Stop");
}

// Called on the ESPHome main loop. is_running() includes STARTING/STOPPING;
// false means the inference task has joined and unloaded its model arenas.
// Never change model state underneath inference. Both models MUST be internal:
// their runtime selection must neither be changed by HA nor saved to flash.
template<class Engine, class Model>
void reconcile_models(Engine *engine, Model *nabu, Model *stop, Mode wanted) {
  if (wanted == Mode::NONE) {
    if (engine->is_running()) engine->stop();
    return;
  }
  const bool want_nabu = wanted == Mode::IDLE_NABU;
  const bool want_stop = wanted == Mode::MUSIC_STOP;
  const bool matching = nabu->is_enabled() == want_nabu && stop->is_enabled() == want_stop;
  if (engine->is_running()) {
    if (!matching) engine->stop();
    return;
  }
  // Disable first, including recovery from an inconsistent both-enabled state.
  if (!want_nabu && nabu->is_enabled()) nabu->disable();
  if (!want_stop && stop->is_enabled()) stop->disable();
  if (want_nabu && !nabu->is_enabled()) nabu->enable();
  if (want_stop && !stop->is_enabled()) stop->enable();
  engine->start();
}
}  // namespace luxe_stop
