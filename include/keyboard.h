#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_err.h"

typedef struct td_keyboard_t {
  i2c_master_dev_handle_t dev;
} td_keyboard_t;

esp_err_t td_keyboard_init(void *ctx);
esp_err_t td_keyboard_poll(void *ctx, uint8_t *c);
#endif /* end of include guard: _KEYBOARD_H_ */
