#ifndef _BOARD_H_
#define _BOARD_H_

#include "driver/i2c_master.h"
#include <battery.h>
#include <display.h>
#include <keyboard.h>
#include <sdcard.h>
#include <speaker.h>
#include <trackball.h>
#include <audio.h>
#include <gps.h>
#include <wifi.h>
#include <jsonconfig.h>

typedef enum {
  INIT_SDCARD = (1<<0),
  INIT_DISPLAY = (1<<1),
  INIT_KEYBOARD = (1<<2),
  INIT_TRACKBALL = (1<<3),
  INIT_SPEAKER = (1<<4),
  INIT_BATTERY = (1<<5),
  INIT_AUDIO = (1<<6),
  INIT_GPS = (1<<7),
  INIT_CONFIG = (1<<8),
  INIT_WIFI = (1<<9)
} td_board_peripherals;

typedef struct td_board_t {
  td_battery_t *Battery;
  td_display_t *Display;
  td_keyboard_t *Keyboard;
  td_trackball_t *Trackball;
  td_speaker_t *Speaker;
  td_sdcard_t *SDCard;
  td_gps_t *Gps;
  td_config_t *Config;
  td_wifi_t *Wifi;
  struct {
    struct {
      i2c_master_bus_handle_t host;
    } i2c;
  } proto;
} td_board_t;

#define BOARD_SPI_MISO_PIN 38
#define BOARD_SPI_MOSI_PIN 41
#define BOARD_SPI_SCK_PIN 40
#define BOARD_SPI SPI2_HOST

#define BOARD_SDCARD_CS_PIN 39
#define BOARD_SDCARD_MOUNT_POINT "/sdcard"
#define BOARD_SDCARD_CLOCK_SPEED 50000

#define BOARD_RADIO_CS_PIN 9
#define BOARD_RADIO_RST_PIN 17
#define BOARD_RADIO_DIO_PIN 45
#define BOARD_RADIO_BUSY_PIN 13
#define BOARD_RADIO_P101_PIN 2



#define BOARD_I2C_SDA_PIN 18
#define BOARD_I2C_SCL_PIN 8
#define BOARD_I2C I2C_NUM_0

#define BOARD_KEYBOARD_ADDR 0x55
#define BOARD_KEYBOARD_SPEED 100000U
#define BOARD_KEYBOARD_POLL_TIMEOUT 100

#define BOARD_BAT_ADC_PIN 4
#define BOARD_BAT_ADC_UNIT ADC_UNIT_1
#define BOARD_BAT_ADC_CHANNEL ADC_CHANNEL_3
#define BOARD_BAT_ADC_ATTEN ADC_ATTEN_DB_12
#define BOARD_BAT_VOLTAGE_MIN 2.5 // 0% battery
#define BOARD_BAT_VOLTAGE_MAX 4.2 // 100% battery

#define BOARD_POWER_PIN 10
#define BOARD_EN_PIN 35

#define BOARD_TRACKBALL_RIGHT_PIN 2
#define BOARD_TRACKBALL_UP_PIN 3
#define BOARD_TRACKBALL_LEFT_PIN 1
#define BOARD_TRACKBALL_DOWN_PIN 15

#define BOARD_TRACKBALL_PINSEL                                                 \
  ((1ULL << BOARD_TRACKBALL_UP_PIN) | (1ULL << BOARD_TRACKBALL_DOWN_PIN) |     \
   (1ULL << BOARD_TRACKBALL_LEFT_PIN) | (1ULL << BOARD_TRACKBALL_RIGHT_PIN))

#define BOARD_SPEAKER_BCLK_PIN 7 // I2S bit clock io number   I2S_BCLK
#define BOARD_SPEAKER_WS_PIN 5   // I2S word select io number    I2S_LRC
// #define BOARD_SPEAKER_DOUT_PIN        6     // I2S data out io number
// I2S_DOUT
#define BOARD_SPEAKER_DIN_PIN 6 // I2S data in io number
#define BOARD_SPEAKER_I2S_NUM I2S_NUM_0
#define BOARD_SPEAKER_DEFAULT_SAMPLERATE SAMPLE_RATE_16K

#define BOARD_DISPLAY_CS_PIN 12
#define BOARD_DISPLAY_BL_PIN 42
#define BOARD_DISPLAY_DC_PIN 11
#define BOARD_DISPLAY_RESET_PIN -1
#define BOARD_DISPLAY_SPEED_HZ SPI_MASTER_FREQ_80M;
#define BOARD_DISPLAY_WIDTH 320
#define BOARD_DISPLAY_HEIGHT 240
#define BOARD_DISPLAY_OFFSETX 0
#define BOARD_DISPLAY_OFFSETY 0

#define BOARD_GPS_TX_PIN 43
#define BOARD_GPS_RX_PIN 44
#define BOARD_GPS_BAUDRATE 38400
#define BOARD_GPS_PORT_NUM UART_NUM_2

#define BOARD_CONFIG_FILE  BOARD_SDCARD_MOUNT_POINT"/config.json"

#endif /* end of include guard: _BOARD_H_ */
