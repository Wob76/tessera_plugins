#include "keg_tap.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

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
    wide_ = context.columns > 1;
    tall_ = context.rows > 1;

    surface_ = lv_obj_get_parent(context.parent);

    beer_ = lv_obj_create(context.parent);
    lv_obj_set_style_radius(beer_, ui::px(2), 0);
    lv_obj_set_style_border_width(beer_, 0, 0);
    lv_obj_set_style_pad_all(beer_, 0, 0);
    glass_ = lv_obj_create(context.parent);
    lv_obj_set_style_radius(glass_, ui::px(2), 0);
    lv_obj_set_style_bg_opa(glass_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(glass_, ui::px(2), 0);
    lv_obj_set_style_pad_all(glass_, 0, 0);
    foam_left_ = lv_obj_create(context.parent);
    foam_right_ = lv_obj_create(context.parent);
    for (lv_obj_t *foam : {foam_left_, foam_right_}) {
      lv_obj_set_style_radius(foam, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_border_width(foam, 0, 0);
      lv_obj_set_style_pad_all(foam, 0, 0);
    }

    title_ = ui::label(context.parent, Font::BODY, theme::MUTED);
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
    if (pouring_) {
      if (wide_ && std::isfinite(rate)) {
        char flow_text[32];
        std::snprintf(flow_text, sizeof(flow_text), "%s %.1f L/min", plugin_->text("pouring"), rate);
        ui::set_text(flow_label_, flow_text);
      } else {
        ui::set_text(flow_label_, plugin_->text("pouring"));
      }
    } else {
      ui::set_text(flow_label_, plugin_->text("ready"));
    }

    const Font count_font = height_ < ui::px(100) && !wide_ && !tall_ ? Font::HEADLINE : Font::VALUE;
    ui::set_font(glasses_label_, count_font);
    const Font volume_font = Font::BODY;
    ui::set_font(volume_label_, volume_font);
    const int title_height = ui::line_height(Font::BODY);
    const int count_height = ui::line_height(count_font);
    const int row_height = ui::line_height(Font::BODY);
    const int pad = ui::px(wide_ ? 12 : 8);
    const int gap = ui::px(4);
    const int icon_width = ui::px(wide_ ? 22 : 17);
    const int icon_height = ui::px(wide_ ? 32 : 25);
    const int icon_x = pad;
    const int icon_y = pad;
    const int title_x = icon_x + icon_width + gap;
    const int header_height = std::max(icon_height, title_height);
    const int content_top = pad + header_height + gap;
    const int content_bottom = height_ - pad;
    const int content_height = std::max(0, content_bottom - content_top);
    const int unit_width = ui::text_width(
        plugin_->text(std::isfinite(count) && std::lround(count) == 1 ? "glass" : "glasses"), Font::BODY);

    lv_obj_set_pos(beer_, icon_x + ui::px(wide_ ? 4 : 3), icon_y + ui::px(wide_ ? 17 : 13));
    lv_obj_set_size(beer_, icon_width - ui::px(wide_ ? 8 : 6), icon_height - ui::px(wide_ ? 20 : 16));
    lv_obj_set_pos(glass_, icon_x, icon_y);
    lv_obj_set_size(glass_, icon_width, icon_height);
    lv_obj_set_pos(foam_left_, icon_x + ui::px(wide_ ? 2 : 1), icon_y + ui::px(wide_ ? 10 : 8));
    lv_obj_set_size(foam_left_, ui::px(wide_ ? 12 : 9), ui::px(wide_ ? 7 : 6));
    lv_obj_set_pos(foam_right_, icon_x + ui::px(wide_ ? 10 : 8), icon_y + ui::px(wide_ ? 8 : 6));
    lv_obj_set_size(foam_right_, ui::px(wide_ ? 11 : 8), ui::px(wide_ ? 8 : 7));
    lv_obj_set_pos(title_, title_x, icon_y + (header_height - title_height) / 2);
    lv_obj_set_size(title_, width_ - title_x - pad, title_height);

    if (wide_) {
      const int left_width = width_ * 48 / 100;
      const int count_block_height = count_height + gap + row_height;
      const int count_y = content_top + std::max(0, (content_height - count_block_height) / 2);
      lv_obj_set_size(glasses_label_, left_width, count_height);
      lv_obj_set_pos(glasses_label_, pad, count_y);
      lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(unit_label_, left_width, row_height);
      lv_obj_set_pos(unit_label_, pad, count_y + count_height + gap);
      lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_CENTER, 0);

      const int details_x = left_width + pad;
      const int details_width = width_ - details_x - pad;
      const int volume_height = ui::line_height(volume_font);
      const int details_height = volume_height + row_height + gap;
      const int details_y = content_top + std::max(0, (content_height - details_height) / 2);
      lv_obj_set_size(volume_label_, details_width, volume_height);
      lv_obj_set_pos(volume_label_, details_x, details_y);
      lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_size(flow_label_, details_width, row_height);
      lv_obj_set_pos(flow_label_, details_x, details_y + volume_height + gap);
      lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_LEFT, 0);
    } else if (tall_) {
      const int count_width = ui::text_width(glasses_text, count_font);
      const int group_width = std::min(width_ - pad * 2, count_width + gap + unit_width);
      const int start_x = (width_ - group_width) / 2;
      const int footer_y = height_ - pad - row_height;
      const int count_y = content_top + std::max(0, (footer_y - content_top - count_height - row_height) / 2);
      const int footer_gap = ui::px(6);
      const int footer_width = std::max(0, (width_ - pad * 2 - footer_gap) / 2);
      lv_obj_set_size(glasses_label_, count_width, count_height);
      lv_obj_set_pos(glasses_label_, start_x, count_y);
      lv_obj_set_size(unit_label_, unit_width, row_height);
      lv_obj_set_pos(unit_label_, start_x + count_width + gap, count_y + count_height - row_height);
      lv_obj_set_size(volume_label_, footer_width, row_height);
      lv_obj_set_pos(volume_label_, pad, footer_y);
      lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_size(flow_label_, footer_width, row_height);
      lv_obj_set_pos(flow_label_, pad + footer_width + footer_gap, footer_y);
      lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_RIGHT, 0);
    } else {
      const int group_width = width_ - pad * 2;
      const int count_block_height = count_height + gap + row_height;
      const int footer_y = content_bottom - row_height;
      const int count_y = content_top + std::max(0, (footer_y - gap - content_top - count_block_height) / 2);
      const int footer_gap = ui::px(6);
      const int footer_width = std::max(0, (group_width - footer_gap) / 2);
      lv_obj_set_size(glasses_label_, group_width, count_height);
      lv_obj_set_pos(glasses_label_, pad, count_y);
      lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(unit_label_, group_width, row_height);
      lv_obj_set_pos(unit_label_, pad, count_y + count_height + gap);
      lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(volume_label_, footer_width, row_height);
      lv_obj_set_pos(volume_label_, pad, footer_y);
      lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(flow_label_, footer_width, row_height);
      lv_obj_set_pos(flow_label_, pad + footer_width + footer_gap, footer_y);
      lv_obj_set_style_text_align(flow_label_, LV_TEXT_ALIGN_CENTER, 0);
    }

    apply_state_colors();
  }

  void on_theme() override {
    apply_state_colors();
  }

 private:
  void apply_state_colors() {
    const theme::Role surface_role = !empty_ && pouring_ ? theme::ACCENT_TINT
                                      : !empty_ && low_stock_ ? theme::AMBER_TRACK
                                                              : theme::CARD;
    const theme::Role primary_text = empty_ ? theme::MUTED : theme::INK;
    const theme::Role secondary_text = empty_ ? theme::SUBTLE : theme::INK_SOFT;
    const theme::Role glass_outline = empty_ ? theme::SUBTLE : theme::MUTED;
    const theme::Role beer_color = empty_ ? theme::TRACK : theme::MARK_AMBER;
    lv_obj_set_style_bg_opa(surface_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(surface_, theme::color(surface_role), 0);
    lv_obj_set_style_border_color(glass_, theme::color(glass_outline), 0);
    lv_obj_set_style_bg_color(beer_, theme::color(beer_color), 0);
    for (lv_obj_t *foam : {foam_left_, foam_right_}) {
      lv_obj_set_style_bg_color(foam, theme::color(secondary_text), 0);
    }
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
  lv_obj_t *glass_{};
  lv_obj_t *beer_{};
  lv_obj_t *foam_left_{};
  lv_obj_t *foam_right_{};
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
