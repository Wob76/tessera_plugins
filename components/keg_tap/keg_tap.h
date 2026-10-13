#pragma once

#include <cmath>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/components/smart_display/plugin_api.h"

namespace esphome::keg_tap {

class KegTap : public Component, public tessera::Plugin {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  bool settings(tessera::SettingsPage &page) override;

  void set_keg1_volume(sensor::Sensor *sensor) { keg1_volume_ = sensor; }
  void set_keg1_glasses(sensor::Sensor *sensor) { keg1_glasses_ = sensor; }
  void set_keg1_flow(binary_sensor::BinarySensor *sensor) { keg1_flow_ = sensor; }
  void set_keg1_label(text_sensor::TextSensor *sensor) { keg1_label_ = sensor; }
  void set_keg1_capacity(number::Number *number) { keg1_capacity_ = number; }
  void set_keg2_volume(sensor::Sensor *sensor) { keg2_volume_ = sensor; }
  void set_keg2_glasses(sensor::Sensor *sensor) { keg2_glasses_ = sensor; }
  void set_keg2_flow(binary_sensor::BinarySensor *sensor) { keg2_flow_ = sensor; }
  void set_keg2_label(text_sensor::TextSensor *sensor) { keg2_label_ = sensor; }
  void set_keg2_capacity(number::Number *number) { keg2_capacity_ = number; }

  sensor::Sensor *volume(bool keg2) const { return keg2 ? keg2_volume_ : keg1_volume_; }
  text_sensor::TextSensor *label(bool keg2) const { return keg2 ? keg2_label_ : keg1_label_; }
  number::Number *capacity(bool keg2) const { return keg2 ? keg2_capacity_ : keg1_capacity_; }
  int full_capacity_liters(bool keg2) const {
    const auto *value = capacity(keg2);
    return value != nullptr && std::isfinite(value->state) ? static_cast<int>(std::lround(value->state)) : 19;
  }
  sensor::Sensor *glasses(bool keg2) const { return keg2 ? keg2_glasses_ : keg1_glasses_; }
  binary_sensor::BinarySensor *flow(bool keg2) const { return keg2 ? keg2_flow_ : keg1_flow_; }
 private:
  void set_full_capacity_liters(bool keg2, int value);

  sensor::Sensor *keg1_volume_{};
  text_sensor::TextSensor *keg1_label_{};
  number::Number *keg1_capacity_{};
  sensor::Sensor *keg1_glasses_{};
  binary_sensor::BinarySensor *keg1_flow_{};
  sensor::Sensor *keg2_volume_{};
  text_sensor::TextSensor *keg2_label_{};
  number::Number *keg2_capacity_{};
  sensor::Sensor *keg2_glasses_{};
  binary_sensor::BinarySensor *keg2_flow_{};
};

}  // namespace esphome::keg_tap
