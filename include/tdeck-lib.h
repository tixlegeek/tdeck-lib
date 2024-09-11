#ifndef _TDECK_LIB_H_
#define _TDECK_LIB_H_


#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#define BOARD_BAT_ADC_PIN 4
#define BOARD_BAT_ADC_UNIT ADC_UNIT_1
#define BOARD_BAT_ADC_CH ADC_CHANNEL_3
#define BOARD_BAT_ADC_ATTEN ADC_ATTEN_DB_12
#define BOARD_POWER_PIN 10


typedef struct {
  adc_oneshot_unit_handle_t dev;
  adc_cali_handle_t cal;
  float voltage;
  bool isCharging;
} td_battery_t;

typedef struct {
  td_battery_t Battery;
} td_board_t;

esp_err_t td_board_init(td_board_t **Board);
esp_err_t td_battery_init(td_board_t *Board);
float td_battery_read(td_board_t *Board);
#endif
/* end of include guard:  _TDECK_LIB_H_ */
