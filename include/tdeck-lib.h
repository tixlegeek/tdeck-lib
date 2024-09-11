#ifndef _TDECK_LIB_H_
#define _TDECK_LIB_H_


#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#define BOARD_BAT_ADC_PIN 4
#define BOARD_BAT_ADC_UNIT ADC_UNIT_1
#define BOARD_BAT_ADC_CHANNEL ADC_CHANNEL_3
#define BOARD_BAT_ADC_ATTEN ADC_ATTEN_DB_12
#define BOARD_POWER_PIN 10
#define BOARD_EN_PIN 35
#define BOARD_BAT_VOLTAGE_MIN 2.5 // 0% battery
#define BOARD_BAT_VOLTAGE_MAX 4.2 // 100% battery

typedef enum {
  TD_BATTERY_CHARGING,
  TD_BATTERY_RANGE_0,
  TD_BATTERY_RANGE_1,
  TD_BATTERY_RANGE_2,
  TD_BATTERY_RANGE_3,
  TD_BATTERY_FULL,
} td_battery_state_t;

typedef struct {
  adc_oneshot_unit_handle_t dev;
  adc_cali_handle_t cal;
  float voltage;
  uint8_t percent;
  bool isCharging;
  uint8_t range;
  td_battery_state_t state;
} td_battery_t;

typedef struct {
  td_battery_t Battery;
} td_board_t;

esp_err_t td_board_init(td_board_t **Board);
esp_err_t td_battery_init(td_board_t *Board);
float td_battery_read(td_board_t *Board);
uint8_t td_battery_voltage2percent(td_board_t *Board, float voltage);
esp_err_t td_battery_update(td_board_t *Board);
#endif
/* end of include guard:  _TDECK_LIB_H_ */
