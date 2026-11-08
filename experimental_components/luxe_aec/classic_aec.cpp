#include "classic_aec.h"

#include <esp_aec.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_rom_sys.h>
#include "esphome/core/log.h"
#include <cstring>
#include <cstdlib>

namespace esphome::luxe_aec {
static const char *const TAG = "luxe_aec";

void ClassicAec::setup() {
  if (this->heap_diagnostics_) this->check_heap_("before aec_create");
  // Low-cost only: do not expose modes whose CPU/RAM requirements are untested
  // on this classic ESP32, or which do not exist in its esp_aec.h.
  this->handle_ = aec_create(16000, this->filter_length_, 1, AEC_MODE_SR_LOW_COST);
  if (this->heap_diagnostics_) this->check_heap_("after aec_create");
  if (this->handle_ == nullptr) {
    ESP_LOGE(TAG, "AEC creation failed; music wake word will remain disabled");
    this->mark_failed();
    return;
  }
  const int chunk = aec_get_chunksize(static_cast<aec_handle_t *>(this->handle_));
  if (chunk <= 0 || chunk > 1024) {
    ESP_LOGE(TAG, "Unsupported AEC frame size: %d", chunk);
    this->release_();
    this->mark_failed();
    return;
  }
  this->frame_size_ = static_cast<size_t>(chunk);
  const size_t bytes = this->frame_size_ * sizeof(int16_t);
  constexpr uint32_t caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
  this->mic_frame_ = static_cast<int16_t *>(heap_caps_aligned_alloc(16, bytes, caps));
  this->ref_frame_ = static_cast<int16_t *>(heap_caps_aligned_alloc(16, bytes, caps));
  this->out_frame_ = static_cast<int16_t *>(heap_caps_aligned_alloc(16, bytes, caps));
  if (!this->mic_frame_ || !this->ref_frame_ || !this->out_frame_) {
    ESP_LOGE(TAG, "AEC aligned-buffer allocation failed; music wake word disabled");
    this->release_();
    this->mark_failed();
  }
  if (this->heap_diagnostics_) this->check_heap_("after frame allocation");
}

void ClassicAec::check_heap_(const char *stage) const {
  // Bypass ESPHome's queued logger: a panic on the other core can otherwise
  // hide the last markers before UART has drained its log queue.
  esp_rom_printf("AEC_HEAP BEGIN: %s\n", stage);
  if (!heap_caps_check_integrity_all(true)) {
    esp_rom_printf("AEC_HEAP FAILED: %s\n", stage);
    std::abort();  // Diagnostic-only, stop before further use of corrupt memory.
  }
  esp_rom_printf("AEC_HEAP OK: %s\n", stage);
}

esp_audio_stack::FrameSpec ClassicAec::frame_spec() const {
  esp_audio_stack::FrameSpec spec;
  spec.input_samples = spec.output_samples = this->frame_size_;
  return spec;
}

bool ClassicAec::process(const int16_t *mic, const int16_t *ref, int16_t *out, uint8_t channels) {
  if (out == nullptr) return false;
  const size_t bytes = this->frame_size_ * sizeof(int16_t);
  if (!this->is_initialized() || mic == nullptr || channels != 1) {
    std::memset(out, 0, bytes);  // Fail closed, never pass uninitialised samples.
    return false;
  }
  if (!this->get_enabled() || this->get_diagnostic_bypass()) {
    std::memcpy(out, mic, bytes);  // Explicit bypass for diagnostic comparisons.
    return true;
  }
  const bool probe = this->heap_diagnostics_ && this->diagnostic_frames_ < 2;
  if (probe) this->check_heap_("before aec_process");
  std::memcpy(this->mic_frame_, mic, bytes);
  if (ref != nullptr) std::memcpy(this->ref_frame_, ref, bytes);
  else std::memset(this->ref_frame_, 0, bytes);
  const int64_t start = esp_timer_get_time();
  aec_process(static_cast<aec_handle_t *>(this->handle_), this->mic_frame_, this->ref_frame_, this->out_frame_);
  const auto elapsed = static_cast<uint32_t>(esp_timer_get_time() - start);
  if (probe) {
    this->check_heap_("after aec_process");
    ++this->diagnostic_frames_;
  }
  std::memcpy(out, this->out_frame_, bytes);
  this->process_us_.store(elapsed, std::memory_order_relaxed);
  // Single audio-task writer; readers only take telemetry snapshots.
  if (elapsed > this->max_us_.load(std::memory_order_relaxed)) this->max_us_.store(elapsed, std::memory_order_relaxed);
  if (elapsed >= this->frame_size_ * 1000000UL / 16000UL) this->overruns_.fetch_add(1, std::memory_order_relaxed);
  this->frames_.fetch_add(1, std::memory_order_relaxed);
  return true;
}

esp_audio_stack::FeatureControl ClassicAec::feature_control(esp_audio_stack::AudioFeature feature) const {
  return feature == esp_audio_stack::AudioFeature::AEC ? esp_audio_stack::FeatureControl::LIVE_TOGGLE
                                                      : esp_audio_stack::FeatureControl::NOT_SUPPORTED;
}
bool ClassicAec::set_feature(esp_audio_stack::AudioFeature feature, bool enabled) {
  if (feature != esp_audio_stack::AudioFeature::AEC) return false;
  this->enabled_.store(enabled, std::memory_order_relaxed);
  return true;
}
esp_audio_stack::ProcessorTelemetry ClassicAec::telemetry() const {
  esp_audio_stack::ProcessorTelemetry value;
  value.frame_count = this->frames_.load(std::memory_order_relaxed);
  value.process_us_last = this->process_us_.load(std::memory_order_relaxed);
  value.process_us_max = this->max_us_.load(std::memory_order_relaxed);
  value.glitch_count = this->overruns_.load(std::memory_order_relaxed);
  return value;
}
void ClassicAec::dump_config() {
  ESP_LOGCONFIG(TAG, "EXPERIMENTAL classic ESP32 AEC: 16 kHz mono, low cost, filter=%d, frame=%u",
                this->filter_length_, static_cast<unsigned>(this->frame_size_));
}
void ClassicAec::dump_diagnostics() const {
  const auto value = this->telemetry();
  ESP_LOGI(TAG, "Diagnostic DSP bypass=%s (capture/wake guards unchanged)",
           this->get_diagnostic_bypass() ? "YES" : "NO");
  ESP_LOGI(TAG, "Ready=%s enabled=%s frame=%u processed=%u DSP=%u us max=%u us overruns=%u internal_free=%u",
           this->is_initialized() ? "YES" : "NO", this->get_enabled() ? "YES" : "NO",
           static_cast<unsigned>(this->frame_size_), static_cast<unsigned>(value.frame_count),
           static_cast<unsigned>(value.process_us_last), static_cast<unsigned>(value.process_us_max),
           static_cast<unsigned>(value.glitch_count),
           static_cast<unsigned>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
}
void ClassicAec::release_() {
  heap_caps_free(this->mic_frame_); this->mic_frame_ = nullptr;
  heap_caps_free(this->ref_frame_); this->ref_frame_ = nullptr;
  heap_caps_free(this->out_frame_); this->out_frame_ = nullptr;
  if (this->handle_) aec_destroy(static_cast<aec_handle_t *>(this->handle_));
  this->handle_ = nullptr;
}
ClassicAec::~ClassicAec() { this->release_(); }
}  // namespace esphome::luxe_aec
