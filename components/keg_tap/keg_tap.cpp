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
    // Explicitly set the radius and clip the surface container to match the card shape
    lv_obj_set_style_radius(surface_, ui::px(14), 0);
    lv_obj_set_style_clip_corner(surface_, true, 0);

    const int segment_count =
        std::clamp(width_ / std::max(1, ui::px(4)), 12, static_cast<int>(MAX_FILL_SEGMENTS));
    fill_count_ = static_cast<size_t>(segment_count);
    for (size_t i = 0; i < fill_count_; ++i) {
      fill_segments_[i] = lv_obj_create(context.parent);
      lv_obj_remove_flag(fill_segments_[i], LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(fill_segments_[i], LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_bg_color(fill_segments_[i], theme::color(theme::AMBER_TRACK), 0);
      lv_obj_set_style_bg_opa(fill_segments_[i], LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(fill_segments_[i], 0, 0);
      lv_obj_set_style_radius(fill_segments_[i], 0, 0);
      lv_obj_set_style_clip_corner(fill_segments_[i], false, 0);
      lv_obj_add_flag(fill_segments_[i], LV_OBJ_FLAG_HIDDEN);
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
    tessera::open_card(plugin_->plugin_id(), "actions", action_entity_, tile_,
                       plugin_->text(keg2_ ? "keg_2_actions" : "keg_1_actions"));
  }

  void on_theme() override {
    apply_state_colors();
  }

 private:
  void update_volume_fill(float volume) {
    const float capacity = capacity_ == nullptr ? NAN : capacity_->state;
    const float fraction = !empty_ && std::isfinite(volume) && std::isfinite(capacity) && capacity > 0.0f
                            ? std::clamp(volume / capacity, 0.0f, 1.0f)
                            : 0.0f;
    const int fill_height = static_cast<int>(std::lround(height_ * fraction));
    const int fill_top = height_ - fill_height;
    const int amplitude = fraction > 0.0f && fraction < 1.0f ? std::min(ui::px(3), fill_height / 2) : 0;

    for (size_t i = 0; i < fill_count_; ++i) {
      const int x_start = static_cast<int>(i * static_cast<size_t>(width_) / fill_count_);
      const int x_end = static_cast<int>((i + 1) * static_cast<size_t>(width_) / fill_count_);
      int wave_offset = 0;
      if (amplitude > 0) {
        const float phase = static_cast<float>(i) * TWO_PI / static_cast<float>(fill_count_);
        wave_offset = static_cast<int>(std::lround(amplitude * std::sin(phase)));
      }
      const int top = std::clamp(fill_top + wave_offset, 0, std::max(0, height_ - 1));
      const int segment_height = height_ - top;
      
      set_hidden(fill_segments_[i], segment_height <= 0);
      if (segment_height > 0) {
        lv_obj_set_pos(fill_segments_[i], x_start, top);
        lv_obj_set_size(fill_segments_[i], x_end - x_start + 1, segment_height);
        lv_obj_set_style_radius(fill_segments_[i], 0, 0);
        lv_obj_set_style_clip_corner(fill_segments_[i], false, 0);
      }
    }
  }

  static void set_hidden(lv_obj_t *obj, bool hidden) {
    const bool is_hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (hidden && !is_hidden) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (!hidden && is_hidden) lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }

  void apply_state_colors() {
    const theme::Role surface_role = !empty_ && pouring_ ? theme::ACCENT_TINT : theme::CARD;
    const theme::Role primary_text = empty_ ? theme::MUTED : theme::INK;
    const theme::Role secondary_text = empty_ ? theme::SUBTLE : theme::INK_SOFT;
    const theme::Role icon_color = empty_ ? theme::MUTED : pouring_ ? theme::ON_ACCENT : theme::ACCENT;
    lv_obj_set_style_bg_opa(surface_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(surface_, theme::color(surface_role), 0);
    for (size_t i = 0; i < fill_count_; ++i) {
      lv_obj_set_style_bg_color(fill_segments_[i], theme::color(theme::AMBER_TRACK), 0);
    }
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
  text_sensor::TextSensor *label_;
  number::Number *capacity_;
  sensor::Sensor *glasses_;
  binary_sensor::BinarySensor *flow_;
  std::string action_entity_;
  int tile_ = -1;
  int width_ = 0;
  int height_ = 0;
  size_t fill_count_ = 0;
  std::array<lv_obj_t *, MAX_FILL_SEGMENTS> fill_segments_{};
  lv_obj_t *surface_{};
  lv_obj_t *icon_label_{};
  lv_obj_t *title_{};
  lv_obj_t *volume_label_{};
  lv_obj_t *glasses_label_{};
  lv_obj_t *unit_label_{};
  lv_obj_t *flow_label_{};
};

class KegActionsCard : public tessera::Card {
 public:
  explicit KegActionsCard(const KegTap *plugin) : plugin_(plugin) {}

  void open(const tessera::CardContext &context) override {
    entity_ = context.entity;
    current_.clear();

    const int margin = ui::px(16);
    const int inset = ui::px(6);
    const int gap = ui::px(4);
    const int option_count = static_cast<int>(OPTION_COUNT);
    const int row_gaps_height = (option_count - 1) * gap;
    const int available_height = std::max(ui::px(1), context.height - margin * 2);
    const int row_height = std::max(
        ui::px(1), std::min(ui::px(48), (available_height - inset * 2 - row_gaps_height) / option_count));
    const int panel_width = std::max(ui::px(1), std::min(context.width - margin * 2, ui::px(440)));
    const int panel_height = inset * 2 + option_count * row_height + row_gaps_height;
    const int panel_x = (context.width - panel_width) / 2;
    const int panel_y = std::max(ui::px(8), (context.height - panel_height) / 2);

    panel_ = ui::block(context.parent, theme::CARD);
    lv_obj_set_size(panel_, panel_width, panel_height);
    lv_obj_set_pos(panel_, panel_x, panel_y);
    lv_obj_set_style_radius(panel_, ui::px(14), 0);

    for (size_t i = 0; i < OPTION_COUNT; ++i) {
      lv_obj_t *row = ui::block(panel_, theme::CARD);
      lv_obj_set_size(row, panel_width - inset * 2, row_height);
      lv_obj_set_pos(row, inset, inset + static_cast<int>(i) * (row_height + gap));
      lv_obj_set_style_radius(row, ui::px(8), 0);
      lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);

      lv_obj_t *label = ui::label(row, Font::BODY_LARGE, theme::INK);
      ui::set_text(label, plugin_->text(OPTIONS[i].label));
      const int label_height = ui::line_height(Font::BODY_LARGE);
      const int side = ui::px(12);
      lv_obj_set_size(label, panel_width - inset * 2 - side * 2, label_height);
      lv_obj_set_pos(label, side, (row_height - label_height) / 2);
      lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);

      lv_obj_t *check = ui::label(row, Font::ICON_SMALL, theme::ACCENT);
      ui::set_text(check, ui::icon(CHECK_ICON));
      const int check_width = ui::text_width(ui::icon(CHECK_ICON), Font::ICON_SMALL);
      const int check_height = ui::line_height(Font::ICON_SMALL);
      lv_obj_set_size(check, check_width, check_height);
      lv_obj_set_pos(check, panel_width - inset * 2 - side - check_width, (row_height - check_height) / 2);
      lv_obj_remove_flag(check, LV_OBJ_FLAG_CLICKABLE);

      rows_[i] = row;
      checks_[i] = check;
      row_contexts_[i] = {this, i};
      lv_obj_add_event_cb(
          row,
          [](lv_event_t *event) {
            auto *row_context = static_cast<RowContext *>(lv_event_get_user_data(event));
            row_context->card->choose(row_context->index);
          },
          LV_EVENT_SHORT_CLICKED, &row_contexts_[i]);
    }
    paint_rows();
  }

  void on_theme() override {
    paint_rows();
  }

 private:
  struct ActionOption {
    const char *value;
    const char *label;
  };
  struct RowContext {
    KegActionsCard *card;
    size_t index;
  };

  static constexpr size_t OPTION_COUNT = 5;
  static constexpr ActionOption OPTIONS[OPTION_COUNT] = {
      {"Idle", "action_idle"},
      {"Zero Scale", "action_zero_scale"},
      {"Calibrate", "action_calibrate"},
      {"Set Empty Keg", "action_set_empty_keg"},
      {"Reset to Defaults", "action_reset_defaults"},
  };

  void choose(size_t index) {
    if (index >= OPTION_COUNT) return;
    if (!tessera::action("select.select_option", entity_, "option", OPTIONS[index].value)) {
      ESP_LOGE(TAG, "Could not send select.select_option to %s with option %s", entity_.c_str(),
               OPTIONS[index].value);
      return;
    }
    ESP_LOGI(TAG, "Requested select.select_option for %s with option %s", entity_.c_str(), OPTIONS[index].value);
    current_ = OPTIONS[index].value;
    paint_rows();
  }

  void paint_rows() {
    lv_obj_set_style_bg_color(panel_, ui::color(theme::CARD), 0);
    for (size_t i = 0; i < OPTION_COUNT; ++i) {
      const bool selected = current_ == OPTIONS[i].value;
      lv_obj_set_style_bg_opa(rows_[i], selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
      lv_obj_set_style_bg_color(rows_[i], ui::color(selected ? theme::ACCENT_TINT : theme::CARD), 0);
      ui::set_color(checks_[i], selected ? theme::ACCENT : theme::CARD);
      set_hidden(checks_[i], !selected);
    }
  }

  static void set_hidden(lv_obj_t *obj, bool hidden) {
    const bool is_hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (hidden && !is_hidden) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (!hidden && is_hidden) lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }

  const KegTap *plugin_;
  std::string entity_;
  std::string current_ = "Idle";
  lv_obj_t *panel_{};
  std::array<lv_obj_t *, OPTION_COUNT> rows_{};
  std::array<lv_obj_t *, OPTION_COUNT> checks_{};
  std::array<RowContext, OPTION_COUNT> row_contexts_{};
};

constexpr KegActionsCard::ActionOption KegActionsCard::OPTIONS[KegActionsCard::OPTION_COUNT];

void KegTap::setup() {
  add_tile("keg1", [this]() { return new KegTile(this, false); });
  add_tile("keg2", [this]() { return new KegTile(this, true); });
  add_card("actions", [this]() { return new KegActionsCard(this); });
}

bool KegTap::settings(tessera::SettingsPage &page) {
  page.number(text("setting_keg1_full_capacity"), 2, 50, 1, " L",
              [this]() { return full_capacity_liters(false); },
              [this](int value) { set_full_capacity_liters(false, value); });
  page.number(text("setting_keg2_full_capacity"), 2, 50, 1, " L",
              [this]() { return full_capacity_liters(true); },
              [this](int value) { set_full_capacity_liters(true, value); });
  return true;
}

void KegTap::set_full_capacity_liters(bool keg2, int value) {
  number::Number *capacity_number = capacity(keg2);
  if (capacity_number == nullptr) {
    ESP_LOGE(TAG, "Cannot set Keg %u capacity: number entity is not configured", keg2 ? 2 : 1);
    return;
  }
  capacity_number->make_call().set_value(static_cast<float>(std::clamp(value, 2, 50))).perform();
}

}  // namespace esphome::keg_tap
