#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/smart_display/plugin_api.h"

namespace esphome::keg_tap {

class KegTap : public Component, public tessera::Plugin {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  bool settings(tessera::SettingsPage &page) override;

  void set_keg1_volume(sensor::Sensor *sensor) { keg1_volume_ = sensor; }
  void set_keg1_glasses(sensor::Sensor *sensor) { keg1_glasses_ = sensor; }
  void set_keg1_pour_rate(sensor::Sensor *sensor) { keg1_pour_rate_ = sensor; }
  void set_keg1_flow(binary_sensor::BinarySensor *sensor) { keg1_flow_ = sensor; }
  void set_keg2_volume(sensor::Sensor *sensor) { keg2_volume_ = sensor; }
  void set_keg2_glasses(sensor::Sensor *sensor) { keg2_glasses_ = sensor; }
  void set_keg2_pour_rate(sensor::Sensor *sensor) { keg2_pour_rate_ = sensor; }
  void set_keg2_flow(binary_sensor::BinarySensor *sensor) { keg2_flow_ = sensor; }

  sensor::Sensor *volume(bool keg2) const { return keg2 ? keg2_volume_ : keg1_volume_; }
  sensor::Sensor *glasses(bool keg2) const { return keg2 ? keg2_glasses_ : keg1_glasses_; }
  sensor::Sensor *pour_rate(bool keg2) const { return keg2 ? keg2_pour_rate_ : keg1_pour_rate_; }
  binary_sensor::BinarySensor *flow(bool keg2) const { return keg2 ? keg2_flow_ : keg1_flow_; }
  float full_capacity_liters(bool keg2) const {
    return static_cast<float>(full_capacity_half_liters_[keg2 ? 1 : 0]) * 0.5f;
  }

 private:
  void set_full_capacity_half_liters(size_t keg_index, int value);

  sensor::Sensor *keg1_volume_{};
  sensor::Sensor *keg1_glasses_{};
  sensor::Sensor *keg1_pour_rate_{};
  binary_sensor::BinarySensor *keg1_flow_{};
  sensor::Sensor *keg2_volume_{};
  sensor::Sensor *keg2_glasses_{};
  sensor::Sensor *keg2_pour_rate_{};
  binary_sensor::BinarySensor *keg2_flow_{};
  int full_capacity_half_liters_[2]{38, 38};
  ESPPreferenceObject full_capacity_preferences_[2];
};

}  // namespace esphome::keg_tap
