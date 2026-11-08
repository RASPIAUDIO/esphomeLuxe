#pragma once

#include <cstdint>

namespace luxe {

enum class ButtonAction { NONE, UP, DOWN, ASSIST };

// Both volume buttons trigger once. Neither release leaks a volume command.
class ButtonChord {
 public:
  ButtonAction update(uint8_t bit, bool pressed) {
    if (pressed) {
      if ((held_ & bit) != 0) return ButtonAction::NONE;
      held_ |= bit;
      if (held_ == 3 && !consumed_) {
        consumed_ = true;
        return ButtonAction::ASSIST;
      }
      return ButtonAction::NONE;
    }
    if ((held_ & bit) == 0) return ButtonAction::NONE;
    held_ &= ~bit;
    const auto action = consumed_ ? ButtonAction::NONE :
        (bit == 1 ? ButtonAction::UP : ButtonAction::DOWN);
    if (held_ == 0) consumed_ = false;
    return action;
  }
 private:
  uint8_t held_{0};
  bool consumed_{false};
};

enum class Phase { IDLE, PREPARING, LISTENING, THINKING, ANSWERING, FINISHING };

class Session {
 public:
  bool begin(bool music_playing) {
    if (active()) return false;
    phase = Phase::PREPARING;
    return_to_music = music_playing;
    pause_sent = false;
    left_group = false;
    return true;
  }
  bool active() const { return phase != Phase::IDLE; }
  bool should_resume() const { return return_to_music && pause_sent; }
  void reset() { *this = Session{}; }
  Phase phase{Phase::IDLE};
  bool return_to_music{false};
  bool pause_sent{false};
  bool left_group{false};
};

inline bool may_wake(bool ha_ready, bool enabled, bool muted, bool session,
                     bool music_playing, bool local_audio, bool speaker_stopped,
                     bool voice_running) {
  return ha_ready && enabled && !muted && !session && !music_playing &&
      !local_audio && speaker_stopped && !voice_running;
}

}  // namespace luxe
