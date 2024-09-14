#include <board.h>
#include <keyboard.h>

const static char *TAG = "KEYBOARD";

esp_err_t td_keyboard_init(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;
  i2c_device_config_t device_config = {
      .scl_speed_hz = BOARD_KEYBOARD_SPEED,
      .device_address = BOARD_KEYBOARD_ADDR,
      .dev_addr_length = I2C_ADDR_BIT_7,
      .scl_wait_us = 0,
      .flags =
          {
              .disable_ack_check = 0,
          },
  };
  return i2c_master_bus_add_device(Board->proto.i2c.host, &device_config,
                                   &Board->Keyboard.dev);
}

esp_err_t td_keyboard_poll(void *ctx, uint8_t *c) {
  td_board_t *Board = (td_board_t *)ctx;
  uint8_t key = 0;
  esp_err_t err = i2c_master_receive(Board->Keyboard.dev, &key, 1,
                                     BOARD_KEYBOARD_POLL_TIMEOUT);
  if (err == ESP_OK && key != '\0') {
    *c = key;
    return ESP_OK;
  }
  return ESP_ERR_TIMEOUT;
}
