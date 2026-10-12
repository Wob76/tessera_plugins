#include "keg_tap.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace esphome::keg_tap {

using tessera::Font;
namespace ui = tessera::ui;

static void style_glass_part(lv_obj_t *obj, theme::Role role) {
  lv_obj_set_style_bg_color(obj, theme::color(role), 0);
  lv_obj_set_style_border_color(obj, theme::color(role), 0);
}

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

    glass_ = lv_obj_create(context.parent);
    lv_obj_set_size(glass_, 18, 20);
    lv_obj_set_style_radius(glass_, ui::px(3), 0);
    lv_obj_set_style_bg_opa(glass_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(glass_, ui::px(2), 0);
    lv_obj_set_style_pad_all(glass_, 0, 0);
    liquid_ = lv_obj_create(context.parent);
    lv_obj_set_size(liquid_, 12, 5);
    lv_obj_set_style_radius(liquid_, ui::px(1), 0);
    lv_obj_set_style_border_width(liquid_, 0, 0);
    lv_obj_set_style_pad_all(liquid_, 0, 0);
    stem_ = lv_obj_create(context.parent);
    lv_obj_set_size(stem_, 3, 5);
    lv_obj_set_style_border_width(stem_, 0, 0);
    lv_obj_set_style_pad_all(stem_, 0, 0);
    base_ = lv_obj_create(context.parent);
    lv_obj_set_size(base_, 12, 2);
    lv_obj_set_style_radius(base_, ui::px(1), 0);
    lv_obj_set_style_border_width(base_, 0, 0);
    lv_obj_set_style_pad_all(base_, 0, 0);

    title_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    lv_obj_set_width(title_, width_ - ui::px(28));
    glasses_label_ = ui::label(context.parent, Font::VALUE, theme::INK);
    lv_obj_set_width(glasses_label_, width_);
    lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
    unit_label_ = ui::label(context.parent, Font::BODY, theme::INK_SOFT);
    volume_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    flow_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    for (lv_obj_t *label : {unit_label_, volume_label_, flow_label_}) {
      lv_obj_set_width(label, width_);
      lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    }
  }

  void on_tick(uint32_t) override {
    ui::set_text(title_, plugin_->text(keg2_ ? "keg_2" : "keg_1"));
    lv_obj_set_style_text_align(title_, LV_TEXT_ALIGN_LEFT, 0);

    char glasses_text[16];
    const float count = glasses_ == nullptr ? NAN : glasses_->state;
    if (std::isfinite(count)) {
      const int whole_count = std::max(0, static_cast<int>(std::lround(count)));
      std::snprintf(glasses_text, sizeof(glasses_text), "%d", whole_count);
      ui::set_text(unit_label_, plugin_->text(whole_count == 1 ? "glass" : "glasses"));
    } else {
      std::snprintf(glasses_text, sizeof(glasses_text), "--");
      ui::set_text(unit_label_, plugin_->text("glasses"));
    }
    ui::set_text(glasses_label_, glasses_text);

    char volume_text[24];
    const float volume = volume_ == nullptr ? NAN : volume_->state;
    if (std::isfinite(volume)) {
      std::snprintf(volume_text, sizeof(volume_text), "%.1f L", volume);
    } else {
      std::snprintf(volume_text, sizeof(volume_text), "-- L");
    }
    ui::set_text(volume_label_, volume_text);

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

    const Font count_font = height_ < ui::px(100) ? Font::HEADLINE : Font::VALUE;
    ui::set_font(glasses_label_, count_font);
    const int title_height = ui::line_height(Font::BODY);
    const int count_height = ui::line_height(count_font);
    const int row_height = ui::line_height(Font::BODY);
    const int gap = ui::px(1);
    const int header_height = ui::px(28);
    const int content_height = header_height + count_height + row_height * 2 + gap * 3;
    int y = (height_ - content_height) / 2;
    if (y < 0) y = 0;

    lv_obj_set_pos(glass_, ui::px(2), y + (title_height - ui::px(27)) / 2);
    lv_obj_set_pos(liquid_, ui::px(5), y + (title_height - ui::px(27)) / 2 + ui::px(12));
    lv_obj_set_pos(stem_, ui::px(9), y + (title_height - ui::px(27)) / 2 + ui::px(20));
    lv_obj_set_pos(base_, ui::px(5), y + (title_height - ui::px(27)) / 2 + ui::px(25));
    lv_obj_set_pos(title_, ui::px(28), y);
    lv_obj_set_height(title_, title_height);
    y += header_height + gap;

    const int unit_width = ui::text_width(unit_label_ == nullptr ? "" : (glasses_ != nullptr && std::isfinite(glasses_->state) &&
                                     std::lround(glasses_->state) == 1 ? plugin_->text("glass") : plugin_->text("glasses")),
                                     Font::BODY);
    const int count_width = ui::text_width(glasses_text, count_font);
    const int group_width = std::min(width_, count_width + ui::px(5) + unit_width);
    const int start_x = (width_ - group_width) / 2;
    lv_obj_set_width(glasses_label_, count_width);
    lv_obj_set_pos(glasses_label_, start_x, y);
    lv_obj_set_height(glasses_label_, count_height);
    lv_obj_set_width(unit_label_, unit_width);
    lv_obj_set_pos(unit_label_, start_x + count_width + ui::px(5), y + count_height - row_height);
    lv_obj_set_height(unit_label_, row_height);
    y += count_height + gap;

    lv_obj_set_pos(volume_label_, 0, y);
    lv_obj_set_height(volume_label_, row_height);
    y += row_height + gap;
    lv_obj_set_pos(flow_label_, 0, y);
    lv_obj_set_height(flow_label_, row_height);
  }

  void on_theme() override {
    style_glass_part(glass_, theme::ACCENT);
    style_glass_part(liquid_, theme::ACCENT);
    style_glass_part(stem_, theme::ACCENT);
    style_glass_part(base_, theme::ACCENT);
    ui::set_color(title_, theme::MUTED);
    ui::set_color(glasses_label_, theme::INK);
    ui::set_color(unit_label_, theme::INK_SOFT);
    ui::set_color(volume_label_, theme::MUTED);
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
  lv_obj_t *glass_{};
  lv_obj_t *liquid_{};
  lv_obj_t *stem_{};
  lv_obj_t *base_{};
  lv_obj_t *title_{};
  lv_obj_t *volume_label_{};
  lv_obj_t *glasses_label_{};
  lv_obj_t *unit_label_{};
  lv_obj_t *flow_label_{};
};

void KegTap::setup() {
  add_tile("keg1", [this]() { return new KegTile(this, false); });
  add_tile("keg2", [this]() { return new KegTile(this, true); });
}

}  // namespace esphome::keg_tap
