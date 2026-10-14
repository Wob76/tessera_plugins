# Changelog

## 0.4.19

- Make the fill more beer like

## 0.4.17

- Fix Rounding on Amber Fill

## 0.4.15

- Drop Pour Rate Sensors to comply with 8 input limit

## 0.4.14

- Expose each full-keg capacity as a persistent Home Assistant config number on the Tessera display.
- Keep the on-device plugin settings and Home Assistant config numbers synchronized to the same values.

## 0.4.13

- Use the keg scale's Home Assistant label entities for tile titles.
- Show full capacity in whole litres and change it in 1 L steps; clip the amber fill to the tile's rounded outline.

## 0.4.12

- Fix the LVGL flag removal calls so the plugin compiles with LVGL's typed flag API.

## 0.4.11

- Add persistent per-keg touchscreen capacity settings and show remaining volume as a bottom-up amber fill.
- Shape the fill level with a wave edge except when full; keep tile text and the pouring tint visible.

## 0.4.10

- Match the volume value's font size to the glasses count on wide tiles; keep the glasses unit at body size.

## 0.4.9

- Match the glasses/unit and volume text size on both wide tile layouts.

## 0.4.8

- Place the glasses count and unit on the left and remaining volume on the right for both wide tile sizes.

## 0.4.7

- Match the 1x2 count-and-unit grouping on both horizontal tile sizes, with volume below.

## 0.4.6

- Use the horizontal space on 2x1 tiles for a large count beside the glasses and volume details.

## 0.4.5

- Keep the glasses count and full detail row visible on horizontal 1x1 and 2x1 tiles.

## 0.4.4

- Keep the selected action highlighted after sending it, and log the requested select entity and option.

## 0.4.3

- Keep the 1x1 detail row in a full-width label to prevent clipping.
- Place the 1x2 glass unit beside the count and keep volume on its own visible row.

## 0.4.2

- Adjust detail layout for 1x1 and 1x2 tiles.
- Use Tessera's always-white on-accent color for the fridge glyph while pouring.

## 0.4.1

- Use a compact detail row for 1x1 and 2x1 tiles so the volume remains visible.
- Remove the tinted icon badge behind the fridge glyph.

## 0.4.0

- Open a keg action menu when a plugin tile is tapped and invoke the configured select entity.

## 0.3.9

- Use Tessera's supported fridge glyph inside the tile.
- Move Ready and pour status to the top right and center the count, unit, and volume in all tile sizes.

## 0.3.8

- Use Tessera's supported fridge icon for the plugin and tile listings.

## 0.3.7

- Replace volume and glass count with a large Empty status when no glasses remain.

## 0.3.6

- Show Empty instead of Ready when no glasses remain.

## 0.3.5

- Space the count and unit together to reduce crowding and balance the wide layout.
- Replace the drawn glass with a circular MDI beer icon that scales by tile size.

## 0.3.4

- Fill the rounded tile with state colors and realign the 1x1 and 2x1 labels to prevent overlap and clipping.

## 0.3.3

- Support 1x2 and 2x2 placements with a tall-tile layout that uses the additional height.

## 0.3.2

- Apply state colors to the tile's actual drawing surface so the pastel backgrounds are visible.
- Show pouring when either the flow binary sensor is on or the pour rate exceeds the flow threshold; keep the compact
  status label short enough to fit.

## 0.3.1

- Dim the tile when no glasses remain, including while the flow sensor is still active.

## 0.3.0

- Replace the stemmed glass with a beer pint icon featuring amber beer and foam.
- Use a dedicated side-by-side layout for 2x1 tiles and keep the status visible in the compact layout.
- Tint the background blue while pouring and amber at five or fewer glasses (pouring takes priority).

## 0.2.0

- Make remaining-glass count the main tile value and add a glass icon.
- Keep the tile display-only; use Tessera's existing Keg Actions select tile to open its native control UI.

## 0.1.0

- Add reusable Keg 1 and Keg 2 tiles for volume, remaining glasses, and active pour rate.
- Allow each screen to select the Home Assistant entities used for both kegs.
