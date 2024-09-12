#include <board.h>
#include <battery.h>

const static char *TAG = "TDECKLIB";

esp_err_t td_battery_init(void *ctx) {
  td_board_t *Board = (td_board_t*)ctx;
  td_battery_t *Battery = (td_battery_t *)(&Board->Battery);
  ESP_LOGI(TAG, "[F] td_battery_init");
  bool calibrated = false;
  esp_err_t ret = ESP_OK;
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = BOARD_BAT_ADC_UNIT,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &Battery->dev));
  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(Battery->dev,
                                             BOARD_BAT_ADC_CHANNEL, &config));

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = BOARD_BAT_ADC_UNIT,
        .chan = BOARD_BAT_ADC_CHANNEL,
        .atten = BOARD_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret =
        adc_cali_create_scheme_curve_fitting(&cali_config, &Battery->cal);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = BOARD_BAT_ADC_UNIT,
        .atten = BOARD_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret =
        adc_cali_create_scheme_line_fitting(&cali_config, &Battery->cal);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Calibration Success");
  } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
    ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
  } else {
    ESP_LOGE(TAG, "Invalid arg or no memory");
  }

  return ESP_OK;
}

float td_battery_read(void *ctx){
  td_board_t *Board = (td_board_t*)ctx;
  td_battery_t *Battery = (td_battery_t *)(&Board->Battery);
  int raw;
  int cal;
  ESP_ERROR_CHECK(adc_oneshot_read(Battery->dev, BOARD_BAT_ADC_CHANNEL, &raw));
  ESP_LOGI(TAG,"ADC%d Channel[%d] Raw: %d", BOARD_BAT_ADC_UNIT + 1, BOARD_BAT_ADC_CHANNEL, raw);
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(Battery->cal, raw, &cal));
  ESP_LOGI(TAG,"ADC%d Channel[%d] Cali: %dmV", BOARD_BAT_ADC_UNIT + 1, BOARD_BAT_ADC_CHANNEL, cal);
  float voltage = (float)(cal * 2.0 / 1000.0);
  return voltage;
}


uint8_t td_battery_voltage2percent(void *ctx, float voltage) {
  td_board_t *Board = (td_board_t*)ctx;
  ESP_LOGI(TAG, "[F] td_battery_voltage2percent");
  if (voltage <= BOARD_BAT_VOLTAGE_MIN) {
    return 0; // Battery is 0%
  } else if (voltage >= BOARD_BAT_VOLTAGE_MAX) {
    return 255; // Battery is 100%
  } else {
    return (uint8_t)((voltage - BOARD_BAT_VOLTAGE_MIN) /
                     (BOARD_BAT_VOLTAGE_MAX - BOARD_BAT_VOLTAGE_MIN) * 100);
  }
}

esp_err_t td_battery_update(void *ctx) {
  td_board_t *Board = (td_board_t*)ctx;
  td_battery_t *Battery = (td_battery_t *)(&Board->Battery);
  ESP_LOGI(TAG, "[F] td_battery_update");
  Battery->voltage = td_battery_read(Board);
  Battery->percent =
      td_battery_voltage2percent(Board, Battery->voltage);
  if (Battery->percent == 255) {
    Battery->state = TD_BATTERY_CHARGING;
    Battery->isCharging = true;
  } else {
    Battery->isCharging = false;
    Battery->range = (uint8_t)(Battery->percent / 20);
    Battery->state =
        (td_battery_state_t)(Battery->range + TD_BATTERY_RANGE_0);
  }

  return ESP_OK;
}
