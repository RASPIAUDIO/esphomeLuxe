#pragma once

#include <cmath>
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::luxe_ip5306 {

// Optional IP5306-I2C on recent Luxe boards. Never changes power settings at boot.
class LuxeIP5306 : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_available(binary_sensor::BinarySensor *s) { available_ = s; }
  void set_charging(binary_sensor::BinarySensor *s) { charging_ = s; }
  void set_full(binary_sensor::BinarySensor *s) { full_ = s; }
  void set_charge_allowed(binary_sensor::BinarySensor *s) { charge_allowed_ = s; }
  void set_level(sensor::Sensor *s) { level_ = s; }
  void set_current_limit(sensor::Sensor *s) { current_limit_ = s; }
  float get_setup_priority() const override { return setup_priority::DATA; }

  void setup() override { update(); }

  void update() override {
    uint8_t control, charge, full, limit, gauge;
    // Address-only ACK probe, then individual register reads. No assumption
    // that the chip supports register auto-increment.
    if (write(nullptr, 0) != i2c::ERROR_OK ||
        !read_byte(0x00, &control) || !read_byte(0x70, &charge) ||
        !read_byte(0x71, &full) || !read_byte(0x24, &limit)) {
      unavailable_();
      return;
    }
    valid_ = true;
    charge_enabled_ = (control & 0x10) != 0;
    available_->publish_state(true);
    charge_allowed_->publish_state(charge_enabled_);
    const bool full_flag = (full & 0x08) != 0;
    full_->publish_state(full_flag);
    charging_->publish_state(charge_enabled_ && (charge & 0x08) && !full_flag);
    // Configured current, NOT measured current. Keep it separate from the
    // charging state: 0x70 alone cannot establish USB presence in every mode.
    current_limit_->publish_state(50 + 100 * (limit & 0x1F));
    // Gauge is optional: undocumented on some revisions. Failure/unknown
    // codes must never be presented as an empty battery.
    float percent = NAN;
    if (read_byte(0x78, &gauge)) {
      switch (gauge >> 4) {
        case 0x0: percent = 100; break;
        case 0x8: percent = 75; break;
        case 0xC: percent = 50; break;
        case 0xE: percent = 25; break;
        case 0xF: percent = 0; break;
      }
    }
    level_->publish_state(percent);
  }

  optional<bool> charge_enabled() const {
    if (!valid_) return {};
    return charge_enabled_;
  }

  void set_charge_enabled(bool enabled) {
    uint8_t before;
    // Read afresh even if the last poll succeeded. Modify only bit 4, leaving
    // boost, key behaviour and all reserved bits untouched.
    if (write(nullptr, 0) != i2c::ERROR_OK || !read_byte(0x00, &before)) {
      unavailable_();
      ESP_LOGW("luxe_ip5306", "Recharge command rejected: circuit unavailable");
      return;
    }
    const uint8_t after = enabled ? (before | 0x10) : (before & ~0x10);
    if (after != before && !write_byte(0x00, after)) {
      unavailable_();
      ESP_LOGW("luxe_ip5306", "Recharge command failed");
      return;
    }
    update();  // Read-back is authoritative; the switch is not optimistic.
    if (valid_ && charge_enabled_ != enabled)
      ESP_LOGW("luxe_ip5306", "Recharge command not confirmed by circuit");
  }

 protected:
  void unavailable_() {
    valid_ = false;
    available_->publish_state(false);
    charging_->invalidate_state();
    full_->invalidate_state();
    charge_allowed_->invalidate_state();
    level_->publish_state(NAN);
    current_limit_->publish_state(NAN);
    // No mark_failed(): an older board must still play audio, and a device
    // temporarily asleep or disconnected must be detected on the next poll.
  }

  binary_sensor::BinarySensor *available_{nullptr};
  binary_sensor::BinarySensor *charging_{nullptr};
  binary_sensor::BinarySensor *full_{nullptr};
  binary_sensor::BinarySensor *charge_allowed_{nullptr};
  sensor::Sensor *level_{nullptr};
  sensor::Sensor *current_limit_{nullptr};
  bool valid_{false};
  bool charge_enabled_{false};
};
}  // namespace esphome::luxe_ip5306

