# Kegerator

Two Tessera tiles for a smart kegerator. Each tile combines a keg's remaining litres, estimated glasses left, and
current pour status. While the keg's flow binary sensor is on, the tile also shows the measured pour rate.

## What you need

- A Tessera screen running Manager 0.4.87 or newer (plugin API 0.8).
- Home Assistant sensors for each keg's remaining volume, remaining glasses, and pour rate, plus a binary flow sensor.
- Volume in litres and pour rate in L/min, matching the units used by this plugin.

## Set up

1. Add this plugin to a screen in Tessera. For a private test, use **Add with a link** with the GitHub repository URL
   after placing these files at that repository's root.
2. In the plugin's screen inputs, select the four Home Assistant entities for each keg.
3. In Layout, add **Keg 1** and/or **Keg 2** from the Plugins group. They can each be used more than once.
4. Choose a 1x1 tile for a compact view or 2x1 for more breathing room.

The default entity IDs match the `keg_scales.yaml` configuration in the smart_kegerator project. Select different
entities in the screen inputs if your Home Assistant IDs differ.

## How it works

- Home Assistant entities are read by ESPHome's native Home Assistant sensor platforms on the screen.
- Tessera's plugin tile reads those sensor states and updates its labels once per second.
- The flow binary sensor gates the pour-rate display, retaining the scale configuration's debounce and delayed-off
  behavior. The tile does not recalculate volume or glass counts.
- The plugin declares only `read_entities` permission. It does not make network requests or call Home Assistant
  services.

## Development

This folder is structured as a standalone Tessera plugin repository. Run Tessera's
`tools/check.py` and `tools/preview.py` against this folder before publishing. API 0.8 is intentional for the
Manager version listed above.
