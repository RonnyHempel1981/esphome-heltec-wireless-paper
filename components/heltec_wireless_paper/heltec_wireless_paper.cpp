#include "heltec_wireless_paper.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace heltec_wireless_paper {

static const char *const TAG = "heltec_wireless_paper";

void HeltecWirelessPaper::setup() {
  this->dc_pin_->setup();
  this->dc_pin_->digital_write(false);

  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
  }

  if (this->busy_pin_ != nullptr) {
    this->busy_pin_->setup();
  }

  this->spi_setup();
  this->init_internal_(this->get_buffer_length_());
  memset(this->buffer_, 0xFF, this->get_buffer_length_());

  this->init_display_();
}

void HeltecWirelessPaper::init_display_() {
  this->reset_();

  // Booster soft start
  this->command_(0x06);
  this->data_(0x17);
  this->data_(0x17);
  this->data_(0x17);

  // Power on
  this->command_(0x04);
  this->wait_busy_(3000);

  // Panel setting: B/W mode, LUT from register, full refresh
  this->command_(0x00);
  this->data_(0xDF);

  // VCOM and data interval: white border
  this->command_(0x50);
  this->data_(0x97);
}

void HeltecWirelessPaper::update() {
  this->do_update_();
  this->send_buffer_();
}

void HeltecWirelessPaper::send_buffer_() {
  bool full_update = (this->update_count_ % this->full_update_every_ == 0);
  this->update_count_++;

  if (full_update) {
    ESP_LOGI(TAG, "Full refresh #%d", this->update_count_);
    this->init_display_();
  }

  // DTM1 - old data (all white)
  this->command_(0x10);
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
    this->data_(0xFF);
    if (i % 1000 == 0)
      App.feed_wdt();
  }

  // DTM2 - new data
  this->command_(0x13);
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
    this->data_(this->buffer_[i]);
    if (i % 1000 == 0)
      App.feed_wdt();
  }

  // Refresh display
  this->command_(0x12);
  this->wait_busy_(10000);
}

void HeltecWirelessPaper::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= VISIBLE_WIDTH || y < 0 || y >= NATIVE_HEIGHT)
    return;

  // Buffer uses NATIVE_WIDTH (128) for row stride, not VISIBLE_WIDTH (122)
  uint32_t pos = x / 8u + y * (NATIVE_WIDTH / 8u);

  if (color.is_on()) {
    this->buffer_[pos] &= ~(0x80 >> (x & 7));
  } else {
    this->buffer_[pos] |= (0x80 >> (x & 7));
  }
}

void HeltecWirelessPaper::fill(Color color) {
  uint8_t fill = color.is_on() ? 0x00 : 0xFF;
  memset(this->buffer_, fill, this->get_buffer_length_());
}

void HeltecWirelessPaper::command_(uint8_t cmd) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(cmd);
  this->disable();
}

void HeltecWirelessPaper::data_(uint8_t val) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_byte(val);
  this->disable();
}

void HeltecWirelessPaper::wait_busy_(uint32_t timeout_ms) {
  if (this->busy_pin_ == nullptr) {
    delay(200);
    return;
  }

  uint32_t start = millis();
  while (this->busy_pin_->digital_read()) {
    if (millis() - start > timeout_ms) {
      ESP_LOGW(TAG, "Busy timeout after %dms!", timeout_ms);
      return;
    }
    App.feed_wdt();
    delay(10);
  }
}

void HeltecWirelessPaper::reset_() {
  if (this->reset_pin_ == nullptr)
    return;

  this->reset_pin_->digital_write(true);
  delay(20);
  this->reset_pin_->digital_write(false);
  delay(20);
  this->reset_pin_->digital_write(true);
  delay(20);
  this->wait_busy_(2000);
}

void HeltecWirelessPaper::dump_config() {
  LOG_DISPLAY("", "Heltec Wireless Paper (JD79656)", this);
  ESP_LOGCONFIG(TAG, "  Visible: %dx%d, RAM: %dx%d, Buffer: %u bytes", VISIBLE_WIDTH, NATIVE_HEIGHT, NATIVE_WIDTH, NATIVE_HEIGHT, this->get_buffer_length_());
  ESP_LOGCONFIG(TAG, "  Full update every: %d", this->full_update_every_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
}

}  // namespace heltec_wireless_paper
}  // namespace esphome
