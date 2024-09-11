#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdio.h>

#include "tdeck-lib.h"

const static char *TAG = "TDECKLIB";

esp_err_t td_board_init(td_board_t **Board) {
  *Board = (td_board_t *)malloc(sizeof(td_board_t));
  if(Board == NULL){
    return ESP_ERR_NO_MEM;
  }
  
  gpio_set_direction(BOARD_POWER_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(BOARD_POWER_PIN, 1);

  return td_battery_init(*Board);
}

esp_err_t td_battery_init(td_board_t *Board) {
  bool calibrated = false;
  td_battery_t *Battery = (td_battery_t *)(&Board->Battery);
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = BOARD_BAT_ADC_UNIT,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &Battery->dev));

  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = BOARD_BAT_ADC_ATTEN,
  };

  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(Battery->dev, BOARD_BAT_ADC_CH, &config));

  esp_err_t ret = ESP_FAIL;
  #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = BOARD_BAT_ADC_UNIT,
        .chan = BOARD_BAT_ADC_CH,
        .atten = BOARD_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &Battery->cal);
    if(ret == ESP_OK){
      calibrated = true;
    }
  #endif
  #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = BOARD_BAT_ADC_UNIT,
        .atten = BOARD_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &Battery->cal);
    if(ret == ESP_OK){
      calibrated = true;
    }
  #endif
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }
  return ret;
}

float td_battery_read(td_board_t *Board){
  td_battery_t *Battery = (td_battery_t *)(&Board->Battery);
  int raw;
  int cal;
  ESP_ERROR_CHECK(adc_oneshot_read(Battery->dev, BOARD_BAT_ADC_CH, &raw));
  ESP_LOGI(TAG,"ADC%d Channel[%d] Raw: %d", BOARD_BAT_ADC_UNIT + 1, BOARD_BAT_ADC_CH, raw);
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(Battery->cal, raw, &cal));
  ESP_LOGI(TAG,"ADC%d Channel[%d] Cali: %dmV", BOARD_BAT_ADC_UNIT + 1, BOARD_BAT_ADC_CH, cal);

  float voltage = (float)(cal * 2.0);
  return voltage;
}
