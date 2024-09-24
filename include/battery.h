#ifndef _TD_BATTERY_H_
#define _TD_BATTERY_H_

#include <esp_err.h>
#include <esp_log.h>
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

typedef enum {
  TD_BATTERY_CHARGING,
  TD_BATTERY_RANGE_0,
  TD_BATTERY_RANGE_1,
  TD_BATTERY_RANGE_2,
  TD_BATTERY_RANGE_3,
  TD_BATTERY_FULL,
} td_battery_state_t;

typedef struct td_battery_t {
  bool initialized;
  bool calibrated;
  adc_oneshot_unit_handle_t dev;
  adc_cali_handle_t cal;
  float voltage;
  uint8_t percent;
  bool isCharging;
  uint8_t range;
  td_battery_state_t state;
} td_battery_t;

esp_err_t td_battery_init(void *ctx);
float td_battery_read(void *ctx);
uint8_t td_battery_voltage2percent(void *ctx, float voltage);
esp_err_t td_battery_update(void *ctx);

#endif /* end of include guard: _TD_BATTERY_H_ */
