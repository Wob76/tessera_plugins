# Kegerator

Two Tessera tiles for a smart kegerator. Each tile combines a keg's remaining litres, estimated glasses left, and
current pour status. A bottom-up amber fill shows the remaining-volume percentage, with a wave-shaped top except when
full. Set each keg's full capacity using the **Keg 1 Full Capacity** and **Keg 2 Full Capacity** config numbers on the
Tessera display device in Home Assistant or on the device under **Settings > Plugins > Kegerator**. Both controls
edit the same value. Each defaults to 19 L, persists across restarts, and is adjustable from 2 to 50 L in 1 L steps.
The amber fill is clipped to the tile's
rounded outline. Tile names come from the keg scale's labels
in Home Assistant. While the keg's flow binary sensor is on, the tile also shows the measured pour rate. The 1x1 and
1x2 sizes place the count beside its glass unit with remaining volume below. Both wide sizes put the count and unit
in the left column and remaining volume in the right; the count and volume values use matching text sizes. Ready or
pour status appears at the top right. The fridge glyph has no background badge and switches to white while pouring for contrast
against the blue tile. Tap a tile to open its keg action menu, matching the
options of the corresponding Home Assistant select entity; the requested action stays checked in the menu after it is
sent. Pouring tints the tile pale blue behind the amber fill. At zero glasses the tile dims and replaces the volume and count with a
large, centered Empty status.

## What you need

- A Tessera screen running Manager 0.4.87 or newer (plugin API 0.8).
- Home Assistant sensors for each keg's remaining volume, remaining glasses, and pour rate, a binary flow sensor,
  and a text label entity (`text.keg_scales_keg_1_scale_label` / `text.keg_scales_keg_2_scale_label` by default).
- The Home Assistant Keg Actions select entities.
- Home Assistant actions enabled for Tessera, so the screen may call `select.select_option`.
- Volume in litres and pour rate in L/min, matching the units used by this plugin.

## Set up

1. Add this plugin to a screen in Tessera. For a private test, use **Add with a link** with the GitHub repository URL
   after placing these files at that repository's root.
2. In the plugin's screen inputs, select the Home Assistant entities for each keg, including its scale-label text
   entity. The label entities are provided by `keg_scales.yaml` and can be renamed from their Home Assistant text
   controls.
3. In Layout, add **Keg 1** and/or **Keg 2** from the Plugins group. They can each be used more than once.
4. Choose a 1x1 tile for a compact view or 2x1 for more breathing room.
5. In each plugin tile's options, confirm the action-select entity. The defaults are
   `select.keg_scales_keg_1_control_action` and `select.keg_scales_keg_2_control_action`; change these if your entity
   IDs differ. Enable Home Assistant actions in Tessera's integration options and allow the plugin's
   `select.select_option` permission when prompted.

The default entity IDs match the `keg_scales.yaml` configuration in the smart_kegerator project. Select different
entities in the screen inputs if your Home Assistant IDs differ.

## How it works

- Home Assistant sensor and text entities are read by ESPHome's native Home Assistant platforms on the screen.
- Tessera's plugin tile reads those sensor states and updates its labels once per second.
- The flow binary sensor gates the pour-rate display, retaining the scale configuration's debounce and delayed-off
  behavior. The displayed pour status also checks the configured pour-rate sensor as a fallback, so it still appears
  if the binary sensor update is delayed or unavailable. The tile does not recalculate volume or glass counts.
- The plugin reads the selected sensors through ESPHome's native Home Assistant sensor platforms. Its tap menu sends
  the selected option to the configured Home Assistant select entity; the existing ESPHome select implements the keg
  calibration and scale actions.
- The fill percentage is remaining volume divided by that keg's full capacity, clamped to 0-100%. Missing or invalid
  volume or capacity readings show no fill. The Home Assistant config number on the display is the source of truth
  for both the fill and its persistent value.
- The tile title follows its selected scale-label text entity. If it is empty or not available, the title falls back to
  Keg 1 or Keg 2.
- At zero or fewer glasses, the tile dims, hides the fill, and replaces its volume and count with a large Empty status.
  While pouring, the card background uses the theme's pale blue tint; otherwise it uses the normal card background.

## Development

This folder is structured as a standalone Tessera plugin repository. Run Tessera's
`tools/check.py` and `tools/preview.py` against this folder before publishing. API 0.8 is intentional for the
Manager version listed above. The Manager's plugin preview displays the metadata example text in a generic tile
layout; it does not render this ESPHome/LVGL implementation, so its geometry and state colors can differ from the
screen. Verify the final appearance on the device.
