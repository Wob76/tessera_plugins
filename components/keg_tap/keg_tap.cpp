#include "keg_tap.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace esphome::keg_tap {

using tessera::Font;
namespace ui = tessera::ui;

class KegTile : public tessera::Tile {
 public:
  KegTile(const KegTap *plugin, bool keg2)
      : plugin_(plugin),
        keg2_(keg2),
        volume_(plugin->volume(keg2)),
        glasses_(plugin->glasses(keg2)),
        pour_rate_(plugin->pour_rate(keg2)),
        flow_(plugin->flow(keg2)) {}

  void create(const tessera::TileContext &context) override {
    width_ = context.width;
    height_ = context.height;

    title_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    volume_label_ = ui::label(context.parent, Font::VALUE, theme::INK);
    glasses_label_ = ui::label(context.parent, Font::BODY, theme::INK);
    flow_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);

    for (lv_obj_t *label : {title_, volume_label_, glasses_label_, flow_label_}) {
      lv_obj_set_width(label, width_);
      lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    }
  }

  void on_tick(uint32_t) override {
    ui::set_text(title_, plugin_->text(keg2_ ? "keg_2" : "keg_1"));

    char volume_text[24];
    const float volume = volume_ == nullptr ? NAN : volume_->state;
    if (std::isfinite(volume)) {
      std::snprintf(volume_text, sizeof(volume_text), "%.1f L", volume);
    } else {
      std::snprintf(volume_text, sizeof(volume_text), "-- L");
    }
    const Font volume_font = ui::text_width(volume_text, Font::VALUE) <= width_ ? Font::VALUE : Font::HEADLINE;
    ui::set_font(volume_label_, volume_font);
    ui::set_text(volume_label_, volume_text);

    char glasses_text[32];
    const float count = glasses_ == nullptr ? NAN : glasses_->state;
    if (std::isfinite(count)) {
      const int whole_count = std::max(0, static_cast<int>(std::lround(count)));
      const char *unit = plugin_->text(whole_count == 1 ? "glass" : "glasses");
      std::snprintf(glasses_text, sizeof(glasses_text), "%d %s", whole_count, unit);
    } else {
      std::snprintf(glasses_text, sizeof(glasses_text), "%s", plugin_->text("unknown_glasses"));
    }
    ui::set_text(glasses_label_, glasses_text);

    if (flow_ != nullptr && flow_->state) {
      const float rate = pour_rate_ == nullptr ? NAN : pour_rate_->state;
      if (std::isfinite(rate)) {
        char flow_text[32];
        std::snprintf(flow_text, sizeof(flow_text), "%s %.1f L/min", plugin_->text("pouring"), rate);
        ui::set_text(flow_label_, flow_text);
      } else {
        ui::set_text(flow_label_, plugin_->text("pouring"));
      }
    } else {
      ui::set_text(flow_label_, plugin_->text("ready"));
    }

    const int title_height = ui::line_height(Font::BODY);
    const int volume_height = ui::line_height(volume_font);
    const int row_height = ui::line_height(Font::BODY);
    const int gap = 2;
    const int content_height = title_height + volume_height + row_height * 2 + gap * 3;
    int y = (height_ - content_height) / 2;
    if (y < 0) y = 0;

    lv_obj_set_pos(title_, 0, y);
    lv_obj_set_height(title_, title_height);
    y += title_height + gap;
    lv_obj_set_pos(volume_label_, 0, y);
    lv_obj_set_height(volume_label_, volume_height);
    y += volume_height + gap;
    lv_obj_set_pos(glasses_label_, 0, y);
    lv_obj_set_height(glasses_label_, row_height);
    y += row_height + gap;
    lv_obj_set_pos(flow_label_, 0, y);
    lv_obj_set_height(flow_label_, row_height);
  }

  void on_theme() override {
    ui::set_color(title_, theme::MUTED);
    ui::set_color(volume_label_, theme::INK);
    ui::set_color(glasses_label_, theme::INK);
    ui::set_color(flow_label_, theme::MUTED);
  }

 private:
  const KegTap *plugin_;
  bool keg2_;
  sensor::Sensor *volume_;
  sensor::Sensor *glasses_;
  sensor::Sensor *pour_rate_;
  binary_sensor::BinarySensor *flow_;
  int width_ = 0;
  int height_ = 0;
  lv_obj_t *title_{};
  lv_obj_t *volume_label_{};
  lv_obj_t *glasses_label_{};
  lv_obj_t *flow_label_{};
};

void KegTap::setup() {
  add_tile("keg1", [this]() { return new KegTile(this, false); });
  add_tile("keg2", [this]() { return new KegTile(this, true); });
}

}  // namespace esphome::keg_tap
