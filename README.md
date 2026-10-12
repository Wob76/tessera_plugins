# Kegerator

Two Tessera tiles for a smart kegerator. Each tile combines a keg's remaining litres, estimated glasses left, and
current pour status. While the keg's flow binary sensor is on, the tile also shows the measured pour rate.
The wide 2x1 layout gives the glass count and volume/status their own columns. The tall 1x2 layout centers the count
and places volume and status side by side along the bottom. Pouring tints the tile pale blue; otherwise, five glasses
or fewer tints it pale amber. At zero glasses the tile dims to show the keg is empty.

## What you need

- A Tessera screen running Manager 0.4.87 or newer (plugin API 0.8).
- Home Assistant sensors for each keg's remaining volume, remaining glasses, and pour rate, plus a binary flow sensor.
- The existing Home Assistant Keg Actions select entities.
- Volume in litres and pour rate in L/min, matching the units used by this plugin.

## Set up

1. Add this plugin to a screen in Tessera. For a private test, use **Add with a link** with the GitHub repository URL
   after placing these files at that repository's root.
2. In the plugin's screen inputs, select the four Home Assistant sensors for each keg.
3. In Layout, add **Keg 1** and/or **Keg 2** from the Plugins group. They can each be used more than once.
4. Choose a 1x1 tile for a compact view or 2x1 for more breathing room.
5. To access keg controls, add the existing `select.keg_scales_keg_1_control_action` or
   `select.keg_scales_keg_2_control_action` as a separate, native Home Assistant tile. Set its **On tap** behavior to
   **Open control**. The plugin tiles are display-only and do not duplicate or invoke keg actions.

The default entity IDs match the `keg_scales.yaml` configuration in the smart_kegerator project. Select different
entities in the screen inputs if your Home Assistant IDs differ.

## How it works

- Home Assistant entities are read by ESPHome's native Home Assistant sensor platforms on the screen.
- Tessera's plugin tile reads those sensor states and updates its labels once per second.
- The flow binary sensor gates the pour-rate display, retaining the scale configuration's debounce and delayed-off
  behavior. The displayed pour status also checks the configured pour-rate sensor as a fallback, so it still appears
  if the binary sensor update is delayed or unavailable. The tile does not recalculate volume or glass counts.
- The plugin reads the selected sensors through ESPHome's native Home Assistant sensor platforms. It does not
  reimplement keg calibration or scale actions; the existing Tessera select tile provides the control UI.
- An empty reading (zero or fewer glasses) dims the text and beer icon and takes priority over the other background
  states. Otherwise, a live pour uses the theme's pale blue tint; at rest, five glasses or fewer use pale amber.
  Other states use the normal card background.

## Development

This folder is structured as a standalone Tessera plugin repository. Run Tessera's
`tools/check.py` and `tools/preview.py` against this folder before publishing. API 0.8 is intentional for the
Manager version listed above.
