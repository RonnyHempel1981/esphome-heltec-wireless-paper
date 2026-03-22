# ESPHome Heltec Wireless Paper Display Component

Custom ESPHome display component for the **Heltec Wireless Paper** board (V1.1+).

The Heltec Wireless Paper uses a 2.13" e-paper display with a **JD79656 controller** (Fitipower) that speaks the UC8151D command set. The standard ESPHome `waveshare_epaper` models all use SSD1680 commands and do **not** work with this display.

## Hardware

| Feature | Details |
|---------|---------|
| MCU | ESP32-S3 |
| Display | 2.13" e-paper (DEPG0213BNS800 / LCMEN2R13EFC1) |
| Controller | JD79656 (UC8151D compatible) |
| Resolution | 250 x 128 (landscape) / 128 x 250 (portrait) |
| Colors | Black / White |
| Interface | SPI |

### Pin Assignment

| Function | GPIO |
|----------|------|
| SPI CLK | GPIO3 |
| SPI MOSI | GPIO2 |
| CS | GPIO4 |
| DC | GPIO5 |
| Reset | GPIO6 |
| Busy | GPIO7 |
| Vext Power | GPIO45 (active LOW) |

> **Important:** GPIO45 (Vext) must be driven LOW to power the display. It is a strapping pin on the ESP32-S3 and requires `ignore_strapping_warning: true`.

## Installation

### As GitHub external component

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/YOUR_USERNAME/YOUR_REPO
      ref: main
    components: [heltec_wireless_paper]
```

### As local component

Copy the `heltec_wireless_paper/` directory into your `components/` folder:

```yaml
external_components:
  - source:
      type: local
      path: components
```

## Configuration

### Minimal example

```yaml
esphome:
  name: my-wireless-paper
  friendly_name: My Wireless Paper
  platformio_options:
    board_build.flash_mode: dio
  on_boot:
    priority: 600.0
    then:
      - output.turn_on: vext_power
      - delay: 200ms

esp32:
  board: esp32-s3-devkitc-1
  variant: esp32s3
  flash_size: 8MB
  framework:
    type: esp-idf

logger:
  hardware_uart: UART0

api:

ota:
  - platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

external_components:
  - source:
      type: local
      path: components

output:
  - platform: gpio
    pin:
      number: GPIO45
      inverted: true
      ignore_strapping_warning: true
    id: vext_power

spi:
  clk_pin: GPIO3
  mosi_pin: GPIO2

font:
  - file: "gfonts://Inter"
    id: font_main
    size: 16

display:
  - platform: heltec_wireless_paper
    id: epaper
    cs_pin: GPIO4
    dc_pin: GPIO5
    reset_pin: GPIO6
    busy_pin: GPIO7
    rotation: 270
    full_update_every: 30
    update_interval: 60s
    lambda: |-
      it.print(0, 0, id(font_main), "Hello Wireless Paper!");
```

### Display configuration options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `cs_pin` | pin | **required** | SPI chip select |
| `dc_pin` | pin | **required** | Data/Command pin |
| `reset_pin` | pin | optional | Hardware reset pin |
| `busy_pin` | pin | optional | Busy status pin |
| `rotation` | int | `0` | Display rotation: 0, 90, 180, 270 |
| `full_update_every` | int | `30` | Full refresh every N updates (reduces ghosting) |
| `update_interval` | time | `60s` | How often to redraw |
| `lambda` | lambda | optional | Drawing code using ESPHome display API |

### Rotation

- `0` - Portrait 128x250
- `90` - Landscape 250x128 (USB port on left)
- `270` - Landscape 250x128 (USB port on right, recommended)

## Notes

- The display uses `board_build.flash_mode: dio` to prevent boot loops on the ESP32-S3.
- `hardware_uart: UART0` is required for serial logging on this board.
- `full_update_every` controls how often a full e-paper refresh cycle is performed. Lower values reduce ghosting but cause more flicker. Higher values use faster partial refreshes.
- All standard ESPHome [display drawing functions](https://esphome.io/components/display/) are supported: `print`, `printf`, `line`, `rectangle`, `circle`, `image`, etc.

## License

MIT
