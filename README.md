# Kegerator

Two Tessera tiles for a smart kegerator. Each tile combines a keg's remaining litres, estimated glasses left, and
current pour status. While the keg's flow binary sensor is on, the tile also shows the measured pour rate. On 1x1, the
count sits above one full-width row with the unit and volume; the 2x1 tile uses the same compact detail row so it
remains visible beneath the count. On 1x2, the unit sits beside the count and volume is below; the taller 2x2 tile
stacks both below the count. Ready or pour status
appears at the top right. The fridge glyph has no background badge and switches to white while pouring for contrast
against the blue tile. Tap a tile to open its keg action menu, matching the
options of the corresponding Home Assistant select entity; the requested action stays checked in the menu after it is
sent. Pouring tints the full rounded tile pale blue; otherwise,
five glasses or fewer tints it pale amber. At zero glasses the tile dims and replaces the volume and count with a
large, centered Empty status.

## What you need

- A Tessera screen running Manager 0.4.87 or newer (plugin API 0.8).
- Home Assistant sensors for each keg's remaining volume, remaining glasses, and pour rate, plus a binary flow sensor.
- The Home Assistant Keg Actions select entities.
- Home Assistant actions enabled for Tessera, so the screen may call `select.select_option`.
- Volume in litres and pour rate in L/min, matching the units used by this plugin.

## Set up

1. Add this plugin to a screen in Tessera. For a private test, use **Add with a link** with the GitHub repository URL
   after placing these files at that repository's root.
2. In the plugin's screen inputs, select the four Home Assistant sensors for each keg.
3. In Layout, add **Keg 1** and/or **Keg 2** from the Plugins group. They can each be used more than once.
4. Choose a 1x1 tile for a compact view or 2x1 for more breathing room.
5. In each plugin tile's options, confirm the action-select entity. The defaults are
   `select.keg_scales_keg_1_control_action` and `select.keg_scales_keg_2_control_action`; change these if your entity
   IDs differ. Enable Home Assistant actions in Tessera's integration options and allow the plugin's
   `select.select_option` permission when prompted.

The default entity IDs match the `keg_scales.yaml` configuration in the smart_kegerator project. Select different
entities in the screen inputs if your Home Assistant IDs differ.

## How it works

- Home Assistant entities are read by ESPHome's native Home Assistant sensor platforms on the screen.
- Tessera's plugin tile reads those sensor states and updates its labels once per second.
- The flow binary sensor gates the pour-rate display, retaining the scale configuration's debounce and delayed-off
  behavior. The displayed pour status also checks the configured pour-rate sensor as a fallback, so it still appears
  if the binary sensor update is delayed or unavailable. The tile does not recalculate volume or glass counts.
- The plugin reads the selected sensors through ESPHome's native Home Assistant sensor platforms. Its tap menu sends
  the selected option to the configured Home Assistant select entity; the existing ESPHome select implements the keg
  calibration and scale actions.
- An empty reading (zero or fewer glasses) dims the tile and replaces its volume and count with a large Empty status.
  This takes priority over the other background states. Otherwise, a live pour uses the theme's pale blue tint; at
  rest, five glasses or fewer use pale amber.
  Other states use the normal card background.

## Development

This folder is structured as a standalone Tessera plugin repository. Run Tessera's
`tools/check.py` and `tools/preview.py` against this folder before publishing. API 0.8 is intentional for the
Manager version listed above. The Manager's plugin preview displays the metadata example text in a generic tile
layout; it does not render this ESPHome/LVGL implementation, so its geometry and state colors can differ from the
screen. Verify the final appearance on the device.
