#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace heltec_wireless_paper {

/// ESPHome display driver for the Heltec Wireless Paper (JD79656 / UC8151D controller).
/// Native resolution: 128 x 250 (portrait). Use rotation: 270 for 250x128 landscape.
///
/// The Heltec Wireless Paper uses a DEPG0213BNS800 / LCMEN2R13EFC1 e-paper panel
/// with a JD79656 controller (Fitipower) that speaks the UC8151D command set.
/// Standard waveshare_epaper models (SSD1680) do NOT work with this display.
///
/// GPIO45 (Vext, active LOW) must be enabled before this component initializes
/// to power the display. Configure it as a gpio output with inverted: true and
/// turn it on during on_boot with priority >= 600.
class HeltecWirelessPaper : public display::DisplayBuffer,
                            public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                  spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_4MHZ> {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void set_dc_pin(GPIOPin *pin) { dc_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { reset_pin_ = pin; }
  void set_busy_pin(GPIOPin *pin) { busy_pin_ = pin; }
  void set_full_update_every(uint32_t val) { full_update_every_ = val; }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }

 protected:
  int get_width_internal() override { return 128; }
  int get_height_internal() override { return 250; }
  uint32_t get_buffer_length_() { return 128u / 8u * 250u; }

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  void fill(Color color) override;

  void init_display_();
  void send_buffer_();
  void command_(uint8_t cmd);
  void data_(uint8_t val);
  void wait_busy_(uint32_t timeout_ms = 5000);
  void reset_();

  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *busy_pin_{nullptr};

  uint32_t full_update_every_{30};
  uint32_t update_count_{0};
};

}  // namespace heltec_wireless_paper
}  // namespace esphome
