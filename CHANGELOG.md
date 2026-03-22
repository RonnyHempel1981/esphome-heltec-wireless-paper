# Changelog

## [1.1.0] - 2026-03-22

### Fixed
- Display now reports correct visible resolution of 122x250 instead of 128x250
- The e-paper panel has 128 columns in controller RAM but only 122 are physically visible
- Previously, with rotation 90/270 the top 6 pixels mapped to invisible RAM area, requiring a manual Y offset workaround
- Buffer row stride remains at 16 bytes (128 bits) to match controller expectations

### Changed
- `get_width_internal()` returns 122 (visible) instead of 128 (RAM)
- Effective display size with rotation 90/270 is now 250x122 instead of 250x128
- Updated README to reflect correct resolution

## [1.0.0] - 2026-03-22

### Added
- Initial release
- Native JD79656/UC8151D driver for Heltec Wireless Paper V1.1+
- Full ESPHome display API support (print, printf, line, rectangle, circle, image, etc.)
- Configurable full/partial refresh cycle via `full_update_every`
- ESP-IDF framework support
- Watchdog feeding during SPI transfers and busy waits
