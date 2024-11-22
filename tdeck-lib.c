#include "tdeck-lib.h"
#include "driver/i2c_master.h"
#include <driver/gpio.h>
#include <esp_psram.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdio.h>

const static char *TAG = "TDECKLIB";

esp_err_t td_board_spi_init(td_board_t *Board) {
  UNUSED(Board);
  spi_bus_config_t bus_config = {
      .miso_io_num = BOARD_SPI_MISO_PIN,
      .mosi_io_num = BOARD_SPI_MOSI_PIN,
      .sclk_io_num = BOARD_SPI_SCK_PIN,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz =  (BOARD_DISPLAY_WIDTH * BOARD_DISPLAY_HEIGHT * 2)
    };
  return spi_bus_initialize(BOARD_SPI, &bus_config, SPI_DMA_CH_AUTO);
}

esp_err_t td_board_i2c_init(td_board_t *Board) {
  assert(Board!=NULL);
  i2c_master_bus_config_t bus_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = BOARD_I2C,
      .scl_io_num = BOARD_I2C_SCL_PIN,
      .sda_io_num = BOARD_I2C_SDA_PIN,
      .glitch_ignore_cnt = 7,
  };
  return i2c_new_master_bus(&bus_config, &Board->proto.i2c.host);
}

td_board_peripherals td_board_init(td_board_t **Board, td_board_peripherals peripherals) {

  if(*Board != NULL){
      ESP_LOGW(TAG, "Already initialized");
      return ESP_OK;
  }

  *Board = (td_board_t *)calloc(sizeof(td_board_t), 1);
  if (Board == NULL) {
    return ESP_ERR_NO_MEM;
  }

  esp_err_t err = ESP_OK;
  td_board_peripherals initialized_peripherals = 0;

  /*size_t psram_size = esp_psram_get_size();
  printf("PSRAM size: %d bytes\n", psram_size);*/

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_POWER_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_POWER_PIN, 1));

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_SDCARD_CS_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_SDCARD_CS_PIN, 1));

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_RADIO_CS_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_RADIO_CS_PIN, 1));

  ESP_ERROR_CHECK(gpio_set_direction(BOARD_DISPLAY_CS_PIN, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(BOARD_DISPLAY_CS_PIN, 1));

  ESP_ERROR_CHECK(td_board_i2c_init(*Board));
  ESP_ERROR_CHECK(td_board_spi_init(*Board));
  gpio_install_isr_service(0);

  if (peripherals & INIT_BATTERY) {
    err = td_battery_init(*Board);
    if(err != ESP_OK){
            ESP_LOGE(TAG, "Battery initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_BATTERY;
  }

  if (peripherals & INIT_DISPLAY) {
    err = td_display_init(*Board);
    if(err != ESP_OK){
            ESP_LOGE(TAG, "Display initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_DISPLAY;
  }

  if (peripherals & INIT_SDCARD) {
    err = td_sdcard_init(*Board, BOARD_SDCARD_MOUNT_POINT);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "SDCard initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }

    err = sdcard_mount(*Board, BOARD_SDCARD_MOUNT_POINT);
    if (err != ESP_OK) {
      //*Board->SDCard.mounted = false;
      ESP_LOGE(TAG, "SDCard not mounted. Err: %s", esp_err_to_name(err));
    } else {
      //*Board->SDCard.mounted = true;
      ESP_LOGI(TAG, "SDCard mounted on \"%s\"", BOARD_SDCARD_MOUNT_POINT);
      initialized_peripherals |= INIT_SDCARD;
    }
  }

  if (peripherals & INIT_KEYBOARD) {
    err = td_keyboard_init(*Board);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "Keyboard initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_KEYBOARD;
  }

  if (peripherals & INIT_SPEAKER) {
    err = td_speaker_init(*Board);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "Speaker initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_SPEAKER;
  }

  if (peripherals & INIT_TRACKBALL) {
    err = td_trackball_init(*Board);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "Trackball initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_TRACKBALL;
  }

  if (peripherals & INIT_GPS) {
    err = td_gps_init(*Board);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "GPS initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_GPS;
  }

  if ((peripherals & INIT_CONFIG) && (initialized_peripherals & INIT_SDCARD)) {
    err = td_config_init(*Board);
    initialized_peripherals &= ~INIT_CONFIG;
    if(err != ESP_OK){
      ESP_LOGE(TAG, "Config initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    ESP_LOGI(TAG,"Config initialized.");
    td_config_t *Config = (*Board)->Config;
    if (td_config_load(BOARD_SDCARD_MOUNT_POINT "/config.json", Config) != ESP_OK) {
      ESP_LOGE(TAG,"Could not load config, rollback on default.");
      if(td_config_default(Config)!=ESP_OK){
        ESP_LOGE(TAG,"Could not load rollback default.");
      }
      else{
        if(td_config_save(BOARD_SDCARD_MOUNT_POINT "/config.json", Config)!=ESP_OK){
          ESP_LOGE(TAG,"Could not save default.");
        }
        else {
          initialized_peripherals |= INIT_CONFIG;
        }
      }
    }
    else  {
      ESP_LOGI(TAG,"Config loaded.");
      initialized_peripherals |= INIT_CONFIG;
    }
  }

  if ((peripherals & INIT_WIFI) && (initialized_peripherals & INIT_CONFIG)) {
    err = td_wifi_init(*Board);
    if(err != ESP_OK){
      ESP_LOGE(TAG, "Config initialisation error: %s", esp_err_to_name(err));
      return ESP_FAIL;
    }
    initialized_peripherals |= INIT_WIFI;
  }

  return initialized_peripherals;
}
