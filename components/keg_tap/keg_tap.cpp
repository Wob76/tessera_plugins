#include "keg_tap.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>

#include "esphome/core/log.h"

namespace esphome::keg_tap {

static const char *const TAG = "keg_tap";
using tessera::Font;
namespace ui = tessera::ui;

constexpr uint32_t FRIDGE_ICON = 0xF0290;
constexpr uint32_t CHECK_ICON = 0xF012C;
constexpr size_t MAX_FILL_SEGMENTS = 24;
constexpr size_t MAX_BUBBLES = 6;
constexpr float TWO_PI = 6.2831853071795864769f;

class KegTile : public tessera::Tile {
 public:
  KegTile(const KegTap *plugin, bool keg2)
      : plugin_(plugin),
        keg2_(keg2),
        volume_(plugin->volume(keg2)),
        label_(plugin->label(keg2)),
        capacity_(plugin->capacity(keg2)),
        glasses_(plugin->glasses(keg2)),
        flow_(plugin->flow(keg2)) {}

  void create(const tessera::TileContext &context) override {
    width_ = context.width;
    height_ = context.height;
    wide_ = context.columns > 1;
    tall_ = context.rows > 1;
    tile_ = context.tile;
    const char *action_entity =
        context.options["action_entity"] |
        (keg2_ ? "select.keg_scales_keg_2_control_action" : "select.keg_scales_keg_1_control_action");
    action_entity_ = action_entity;

    surface_ = lv_obj_get_parent(context.parent);

    const int segment_count =
        std::clamp(width_ / std::max(1, ui::px(4)), 12, static_cast<int>(MAX_FILL_SEGMENTS));
    fill_count_ = static_cast<size_t>(segment_count);
    
    // Create amber body segments (flat bottom, wavy top)
    for (size_t i = 0; i < fill_count_; ++i) {
      fill_segments_[i] = lv_obj_create(context.parent);
      lv_obj_remove_flag(fill_segments_[i], LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(fill_segments_[i], LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_bg_color(fill_segments_[i], theme::color(theme::AMBER_TRACK), 0);
      lv_obj_set_style_bg_opa(fill_segments_[i], LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(fill_segments_[i], 0, 0);
      lv_obj_set_style_radius(fill_segments_[i], 0, 0);
      lv_obj_add_flag(fill_segments_[i], LV_OBJ_FLAG_HIDDEN);
    }

    // Create foam head segments with semi-transparency so text remains readable
    for (size_t i = 0; i < fill_count_; ++i) {
      foam_segments_[i] = lv_obj_create(context.parent);
      lv_obj_remove_flag(foam_segments_[i], LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(foam_segments_[i], LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_bg_color(foam_segments_[i], lv_color_hex(0xE2DDC9), 0);
      lv_obj_set_style_bg_opa(foam_segments_[i], LV_OPA_70, 0);
      lv_obj_set_style_border_width(foam_segments_[i], 0, 0);
      lv_obj_set_style_radius(foam_segments_[i], ui::px(2), 0);
      lv_obj_add_flag(foam_segments_[i], LV_OBJ_FLAG_HIDDEN);
    }

    // Create subtle carbonation bubbles inside the beer
    for (size_t i = 0; i < MAX_BUBBLES; ++i) {
      bubbles_[i] = lv_obj_create(context.parent);
      lv_obj_remove_flag(bubbles_[i], LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(bubbles_[i], LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_bg_color(bubbles_[i], lv_color_hex(0xE2DDC9), 0);
      lv_obj_set_style_bg_opa(bubbles_[i], LV_OPA_30, 0);
      lv_obj_set_style_border_width(bubbles_[i], 0, 0);
      lv_obj_set_style_radius(bubbles_[i], LV_RADIUS_CIRCLE, 0);
      lv_obj_set_size(bubbles_[i], ui::px(3), ui::px(3));
      lv_obj_add_flag(bubbles_[i], LV_OBJ_FLAG_HIDDEN);
    }

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
    const char *fallback_title = plugin_->text(keg2_ ? "keg_2" : "keg_1");
    const std::string label = label_ == nullptr ? "" : label_->state;
    ui::set_text(title_, label.empty() ? fallback_title : label.c_str());
    lv_obj_set_style_text_align(title_, LV_TEXT_ALIGN_LEFT, 0);
    ui::set_text(icon_label_, ui::icon(FRIDGE_ICON));
    const Font icon_font = tall_ ? Font::ICON : Font::ICON_SMALL;
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
    update_volume_fill(volume);

    pouring_ = (flow_ != nullptr && flow_->state);
    low_stock_ = std::isfinite(count) && std::lround(count) <= 5;
    char pouring_text[48];
    const char *status_text;
    if (empty_) {
      status_text = plugin_->text("empty");
    } else if (pouring_) {
      status_text = plugin_->text("pouring");
    } else {
      status_text = plugin_->text("ready");
    }
    ui::set_text(flow_label_, status_text);

    const Font count_font = !tall_ && !wide_ ? Font::HEADLINE : Font::VALUE;
    ui::set_font(glasses_label_, count_font);
    const int title_height = ui::line_height(Font::BODY);
    const int count_height = ui::line_height(count_font);
    const int row_height = ui::line_height(Font::BODY);
    const int pad = ui::px(wide_ ? 12 : 8);
    const int gap = ui::px(4);
    const int icon_slot_size = ui::px(wide_ && tall_ ? 44 : tall_ ? 36 : 28);
    const int icon_font_size = ui::text_width(ui::icon(FRIDGE_ICON), icon_font);
    const int icon_font_height = ui::line_height(icon_font);
    const int icon_x = pad;
    const int icon_y = pad;
    const int title_x = icon_x + icon_slot_size + gap;
    const int header_height = std::max(icon_slot_size, title_height);
    const int content_top = pad + header_height + gap;
    const int content_bottom = height_ - pad;
    const int content_height = std::max(0, content_bottom - content_top);
    set_hidden(glasses_label_, empty_);
    set_hidden(unit_label_, empty_);
    set_hidden(volume_label_, empty_);

    lv_obj_set_size(icon_label_, icon_font_size, icon_font_height);
    lv_obj_set_pos(icon_label_, icon_x + (icon_slot_size - icon_font_size) / 2,
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
      const bool narrow_tall_layout = !wide_ && tall_;
      const bool stacked_layout = wide_ && tall_;
      const int detail_gap = ui::px(8);
      const char *unit_text = plugin_->text(std::isfinite(count) && std::lround(count) == 1 ? "glass" : "glasses");
      const int volume_height = row_height;
      const int count_row_height = std::max(count_height, row_height);
      const int stack_height =
          count_row_height + gap + row_height + (stacked_layout ? gap + volume_height : 0);
      const int stack_y = content_top + std::max(0, (content_height - stack_height) / 2);
      lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_size(glasses_label_, content_width, count_height);
      if (wide_) {
        const Font wide_detail_font = Font::VALUE;
        const int wide_detail_height = ui::line_height(wide_detail_font);
        const int detail_column_gap = ui::px(12);
        const int column_width = (content_width - detail_column_gap) / 2;
        const int count_width = ui::text_width(glasses_text, count_font) + ui::px(8);
        const int unit_width = ui::text_width(unit_text, Font::BODY) + ui::px(8);
        const int group_width = count_width + detail_gap + unit_width;
        const int group_x = pad + std::max(0, (column_width - group_width) / 2);
        const int wide_row_height = std::max(count_height, wide_detail_height);
        const int row_y = content_top + std::max(0, (content_height - wide_row_height) / 2);
        const int volume_x = pad + column_width + detail_column_gap;
        const int volume_width = content_width - column_width - detail_column_gap;
        lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_size(glasses_label_, count_width, count_height);
        lv_obj_set_pos(glasses_label_, group_x, row_y + (wide_row_height - count_height) / 2);
        ui::set_font(unit_label_, Font::BODY);
        lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_size(unit_label_, unit_width, row_height);
        const int unit_y = row_y + (wide_row_height - row_height) / 2;
        lv_obj_set_pos(unit_label_, group_x + count_width + detail_gap, unit_y);
        ui::set_font(volume_label_, wide_detail_font);
        lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(volume_label_, volume_width, wide_detail_height);
        lv_obj_set_pos(volume_label_, volume_x, row_y + (wide_row_height - wide_detail_height) / 2);
      } else if (!tall_) {
        const int count_width = ui::text_width(glasses_text, count_font) + ui::px(10);
        const int unit_width = ui::text_width(unit_text, Font::BODY) + ui::px(10);
        const int group_width = count_width + detail_gap + unit_width;
        const int group_x = pad + std::max(0, (content_width - group_width) / 2);
        const int group_height = count_row_height + gap + volume_height;
        const int group_y = content_top + std::max(0, (content_height - group_height) / 2);
        lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_size(glasses_label_, count_width, count_height);
        lv_obj_set_pos(glasses_label_, group_x, group_y + (count_row_height - count_height) / 2);
        lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_size(unit_label_, unit_width, row_height);
        const int unit_y = group_y + (count_row_height - row_height) / 2;
        lv_obj_set_pos(unit_label_, group_x + count_width + detail_gap, unit_y);
        lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(volume_label_, content_width, volume_height);
        lv_obj_set_pos(volume_label_, pad, group_y + count_row_height + gap);
      } else if (narrow_tall_layout) {
        const int count_width = ui::text_width(glasses_text, count_font) + ui::px(4);
        const int unit_width = ui::text_width(unit_text, Font::BODY) + ui::px(4);
        const int group_width = count_width + detail_gap + unit_width;
        const int group_x = pad + std::max(0, (content_width - group_width) / 2);
        lv_obj_set_style_text_align(glasses_label_, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_size(glasses_label_, count_width, count_height);
        lv_obj_set_pos(glasses_label_, group_x, stack_y + (count_row_height - count_height) / 2);
        lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_size(unit_label_, unit_width, row_height);
        lv_obj_set_pos(unit_label_, group_x + count_width + detail_gap, stack_y + (count_row_height - row_height) / 2);
        lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(volume_label_, content_width, volume_height);
        lv_obj_set_pos(volume_label_, pad, stack_y + count_row_height + gap);
      } else {
        lv_obj_set_pos(glasses_label_, pad, stack_y);
        lv_obj_set_style_text_align(unit_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(unit_label_, content_width, row_height);
        lv_obj_set_pos(unit_label_, pad, stack_y + count_height + gap);
        lv_obj_set_style_text_align(volume_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(volume_label_, content_width, volume_height);
        lv_obj_set_pos(volume_label_, pad, stack_y + count_height + gap + row_height + gap);
      }
    }

    apply_state_colors();
  }

  void on_tap() override {
