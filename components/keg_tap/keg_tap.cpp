#include "keg_tap.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace esphome::keg_tap {

using tessera::Font;
namespace ui = tessera::ui;

constexpr uint32_t FRIDGE_ICON = 0xF0290;

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
    wide_ = context.columns > 1;
    tall_ = context.rows > 1;

    surface_ = lv_obj_get_parent(context.parent);

    icon_circle_ = ui::block(context.parent, theme::ACCENT_TINT);
    lv_obj_set_style_radius(icon_circle_, LV_RADIUS_CIRCLE, 0);

    title_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    icon_label_ = ui::label(context.parent, Font::ICON, theme::ACCENT);
    glasses_label_ = ui::label(context.parent, Font::VALUE, theme::INK);
    lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
    unit_label_ = ui::label(context.parent, Font::BODY, theme::INK_SOFT);
    volume_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    flow_label_ = ui::label(context.parent, Font::BODY, theme::MUTED);
    lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_CENTER, 0);
  }

  void on_tick(uint32_t) override {
    ui::set_text(title_, plugin_->text(keg2_ ? "keg_2" : "keg_1"));
    lv_obj_set_style_text_align(title_, LV_TEXT_ALIGN_LEFT, 0);
    ui::set_text(icon_label_, ui::icon(FRIDGE_ICON));
    const Font icon_font = wide_ || tall_ ? Font::ICON : Font::ICON_SMALL;
    ui::set_font(icon_label_, icon_font);
    lv_obj_set_style_text_align(icon_label_, LV_TEXT_ALIGN_CENTER, 0);

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
    empty_ = std::isfinite(count) && std::lround(count) <= 0;

    char volume_text[24];
    const float volume = volume_ == nullptr ? NAN : volume_->state;
    if (std::isfinite(volume)) {
      std::snprintf(volume_text, sizeof(volume_text), "%.1f L", volume);
    } else {
      std::snprintf(volume_text, sizeof(volume_text), "-- L");
    }
    ui::set_text(volume_label_, volume_text);

    const float rate = pour_rate_ == nullptr ? NAN : pour_rate_->state;
    pouring_ = (flow_ != nullptr && flow_->state) || (std::isfinite(rate) && rate > 0.25f);
    low_stock_ = std::isfinite(count) && std::lround(count) <= 5;
    char pouring_text[48];
    const char *status_text;
    if (empty_) {
      status_text = plugin_->text("empty");
    } else if (pouring_) {
      if (wide_ && std::isfinite(rate)) {
        std::snprintf(pouring_text, sizeof(pouring_text), "%s %.1f L/min", plugin_->text("pouring"), rate);
        status_text = pouring_text;
      } else {
        status_text = plugin_->text("pouring");
      }
    } else {
      status_text = plugin_->text("ready");
    }
    ui::set_text(flow_label_, status_text);

    const Font count_font = height_ < ui::px(100) && !wide_ && !tall_ ? Font::HEADLINE : Font::VALUE;
    ui::set_font(glasses_label_, count_font);
    const int title_height = ui::line_height(Font::BODY);
    const int count_height = ui::line_height(count_font);
    const int row_height = ui::line_height(Font::BODY);
    const int pad = ui::px(wide_ ? 12 : 8);
    const int gap = ui::px(4);
    const int icon_size = ui::px(wide_ && tall_ ? 44 : wide_ || tall_ ? 36 : 28);
    const int icon_font_size = ui::text_width(ui::icon(FRIDGE_ICON), icon_font);
    const int icon_font_height = ui::line_height(icon_font);
    const int icon_x = pad;
    const int icon_y = pad;
    const int title_x = icon_x + icon_size + gap;
    const int header_height = std::max(icon_size, title_height);
    const int content_top = pad + header_height + gap;
    const int content_bottom = height_ - pad;
    const int content_height = std::max(0, content_bottom - content_top);
    set_hidden(glasses_label_, empty_);
    set_hidden(unit_label_, empty_);
    set_hidden(volume_label_, empty_);

    lv_obj_set_size(icon_circle_, icon_size, icon_size);
    lv_obj_set_pos(icon_circle_, icon_x, icon_y + (header_height - icon_size) / 2);
    lv_obj_set_size(icon_label_, icon_font_size, icon_font_height);
    lv_obj_set_pos(icon_label_, icon_x + (icon_size - icon_font_size) / 2,
                   icon_y + (header_height - icon_font_height) / 2);
    const int status_width = ui::text_width(status_text, Font::BODY);
    const int status_x = std::max(title_x + gap, width_ - pad - status_width);
    const int title_width = std::max(0, status_x - title_x - gap);
    lv_obj_set_pos(title_, title_x, icon_y + (header_height - title_height) / 2);
    lv_obj_set_size(title_, title_width, title_height);

    if (empty_) {
      const int status_height = ui::line_height(Font::HEADLINE);
      ui::set_font(flow_label_, Font::HEADLINE);
      lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(flow_label_, width_ - pad * 2, status_height);
      lv_obj_set_pos(flow_label_, pad, content_top + std::max(0, (content_height - status_height) / 2));
    } else {
      ui::set_font(flow_label_, Font::BODY);
      lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_RIGHT, 0);
      lv_obj_set_size(flow_label_, width_ - status_x - pad, row_height);
      lv_obj_set_pos(flow_label_, status_x, icon_y + (header_height - row_height) / 2);

      const int content_width = width_ - pad * 2;
      const int volume_height = row_height;
      const int stack_height = count_height + gap + row_height + gap + volume_height;
      const int stack_y = content_top + std::max(0, (content_height - stack_height) / 2);
      lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(glasses_label_, content_width, count_height);
      lv_obj_set_pos(glasses_label_, pad, stack_y);
      lv_obj_set_size(unit_label_, content_width, row_height);
      lv_obj_set_pos(unit_label_, pad, stack_y + count_height + gap);
      lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(volume_label_, content_width, volume_height);
      lv_obj_set_pos(volume_label_, pad, stack_y + count_height + gap + row_height + gap);
    }

    apply_state_colors();
  }

  void on_theme() override {
    apply_state_colors();
  }

 private:
  static void set_hidden(lv_obj_t *obj, bool hidden) {
    const bool is_hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (hidden && !is_hidden) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (!hidden && is_hidden) lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }

  void apply_state_colors() {
    const theme::Role surface_role = !empty_ && pouring_ ? theme::ACCENT_TINT
                                      : !empty_ && low_stock_ ? theme::AMBER_TRACK
                                                              : theme::CARD;
    const theme::Role primary_text = empty_ ? theme::MUTED : theme::INK;
    const theme::Role secondary_text = empty_ ? theme::SUBTLE : theme::INK_SOFT;
    const theme::Role icon_background = empty_ ? theme::TRACK : theme::ACCENT_TINT;
    const theme::Role icon_color = empty_ ? theme::MUTED : theme::ACCENT;
    lv_obj_set_style_bg_opa(surface_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(surface_, theme::color(surface_role), 0);
    lv_obj_set_style_bg_color(icon_circle_, theme::color(icon_background), 0);
    lv_obj_set_style_text_color(icon_label_, theme::color(icon_color), 0);
    ui::set_color(title_, empty_ ? theme::SUBTLE : theme::MUTED);
    ui::set_color(glasses_label_, primary_text);
    ui::set_color(unit_label_, secondary_text);
    ui::set_color(volume_label_, secondary_text);
    ui::set_color(flow_label_, empty_ ? theme::SUBTLE
                                     : pouring_ ? theme::ACCENT
                                                : low_stock_ ? theme::MARK_AMBER : theme::MUTED);
  }

  const KegTap *plugin_;
  bool keg2_;
  bool wide_ = false;
  bool tall_ = false;
  bool pouring_ = false;
  bool low_stock_ = false;
  bool empty_ = false;
  sensor::Sensor *volume_;
  sensor::Sensor *glasses_;
  sensor::Sensor *pour_rate_;
  binary_sensor::BinarySensor *flow_;
  int width_ = 0;
  int height_ = 0;
  lv_obj_t *surface_{};
  lv_obj_t *icon_circle_{};
  lv_obj_t *icon_label_{};
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
