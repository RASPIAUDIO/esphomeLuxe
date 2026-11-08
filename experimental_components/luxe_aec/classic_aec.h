#pragma once

#include "esphome/components/esp_audio_stack/audio_core_processor.h"
#include "esphome/core/component.h"
#include <atomic>

namespace esphome::luxe_aec {

// Uses the classic ESP32 aec_* API; the Radio's afe_aec_* adapter is not reused.
class ClassicAec final : public Component, public esp_audio_stack::AudioProcessor {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }
  void set_filter_length(int length) { this->filter_length_ = length; }
  void set_heap_diagnostics(bool enabled) { this->heap_diagnostics_ = enabled; }
  bool is_initialized() const override { return this->handle_ != nullptr && !this->is_failed(); }
  esp_audio_stack::FrameSpec frame_spec() const override;
  bool process(const int16_t *mic, const int16_t *ref, int16_t *out, uint8_t channels) override;
  esp_audio_stack::FeatureControl feature_control(esp_audio_stack::AudioFeature feature) const override;
  bool set_feature(esp_audio_stack::AudioFeature feature, bool enabled) override;
  esp_audio_stack::ProcessorTelemetry telemetry() const override;
  bool reconfigure(int, int) override { return false; }
  bool get_enabled() const { return this->enabled_.load(std::memory_order_relaxed); }
  // Diagnostic only: keep capture/wake eligibility but skip DSP, to isolate load.
  void set_diagnostic_bypass(bool bypass) { this->diagnostic_bypass_.store(bypass, std::memory_order_relaxed); }
  bool get_diagnostic_bypass() const { return this->diagnostic_bypass_.load(std::memory_order_relaxed); }
  bool ready_for_music_wake() const {
    return this->is_initialized() && this->get_enabled();
  }
  void dump_diagnostics() const;
  ~ClassicAec() override;

 protected:
  void release_();
  void check_heap_(const char *stage) const;
  bool heap_diagnostics_{false};
  uint8_t diagnostic_frames_{0};
  void *handle_{nullptr};
  int16_t *mic_frame_{nullptr};
  int16_t *ref_frame_{nullptr};
  int16_t *out_frame_{nullptr};
  size_t frame_size_{512};
  int filter_length_{2};
  std::atomic<bool> enabled_{true};
  std::atomic<bool> diagnostic_bypass_{false};
  std::atomic<uint32_t> frames_{0};
  std::atomic<uint32_t> process_us_{0};
  std::atomic<uint32_t> max_us_{0};
  std::atomic<uint32_t> overruns_{0};
};

}  // namespace esphome::luxe_aec
