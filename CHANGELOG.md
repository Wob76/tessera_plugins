# Changelog

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
