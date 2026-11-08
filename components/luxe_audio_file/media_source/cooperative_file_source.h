#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32
#include "cooperative_write.h"
#include "esphome/components/audio_file/media_source/audio_file_media_source.h"
#include "esphome/components/media_source/media_source.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::luxe_audio_file {

// Compose the native decoder rather than replacing it. Only local UI cues use
// this adapter; Sendspin and the Voice Answer decoder keep their native paths.
class CooperativeFileSource final : public Component,
                                    public media_source::MediaSource,
                                    public media_source::MediaSourceListener {
 public:
  void set_source(audio_file::AudioFileMediaSource *source) { this->source_ = source; }
  void setup() override { this->source_->set_listener(this); }

  bool can_handle(const std::string &uri) const override { return this->source_->can_handle(uri); }
  bool play_uri(const std::string &uri) override {
    this->waits_.store(0, std::memory_order_relaxed);
    this->bytes_.store(0, std::memory_order_relaxed);
    return this->source_->play_uri(uri);
  }
  void handle_command(media_source::MediaSourceCommand command) override {
    this->source_->handle_command(command);
  }

  size_t write_audio(const uint8_t *data, size_t length, uint32_t timeout_ms,
                     const audio::AudioStreamInfo &info) override {
    const size_t accepted = luxe::cooperative_write(
        [&]() { return this->write_output(data, length, timeout_ms, info); },
        [&]() {
          this->waits_.fetch_add(1, std::memory_order_relaxed);
          vTaskDelay(1);  // One real scheduler tick, even if pdMS_TO_TICKS(1) == 0.
        });
    this->bytes_.fetch_add(accepted, std::memory_order_relaxed);
    return accepted;
  }

  void report_state(media_source::MediaSourceState state) override {
    if (state == media_source::MediaSourceState::IDLE) {
      ESP_LOGI("ui_sound", "Cue decoded: %u PCM bytes, %u cooperative waits",
               static_cast<unsigned>(this->bytes_.load(std::memory_order_relaxed)),
               static_cast<unsigned>(this->waits_.load(std::memory_order_relaxed)));
    }
    this->set_state_(state);
  }

 protected:
  audio_file::AudioFileMediaSource *source_{nullptr};
  std::atomic<uint32_t> waits_{0};
  std::atomic<uint32_t> bytes_{0};
};

}  // namespace esphome::luxe_audio_file
#endif
