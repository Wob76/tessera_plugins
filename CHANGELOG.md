# Changelog

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
