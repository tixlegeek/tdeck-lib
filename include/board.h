#ifndef _BOARD_H_
#define _BOARD_H_

#include <battery.h>
#include <display.h>

typedef struct td_board_t {
  td_battery_t Battery;
  td_display_t Display;
} td_board_t;

#define BOARD_SPI_MISO_PIN 38
#define BOARD_SPI_MOSI_PIN 41
#define BOARD_SPI_SCK_PIN 40
#define BOARD_SPI SPI2_HOST
#define BOARD_BAT_ADC_PIN 4
#define BOARD_BAT_ADC_UNIT ADC_UNIT_1
#define BOARD_BAT_ADC_CHANNEL ADC_CHANNEL_3
#define BOARD_BAT_ADC_ATTEN ADC_ATTEN_DB_12
#define BOARD_POWER_PIN 10
#define BOARD_EN_PIN 35
#define BOARD_BAT_VOLTAGE_MIN 2.5 // 0% battery
#define BOARD_BAT_VOLTAGE_MAX 4.2 // 100% battery

#define BOARD_DISPLAY_CS_PIN 12
#define BOARD_DISPLAY_BL_PIN 42
#define BOARD_DISPLAY_DC_PIN 11
#define BOARD_DISPLAY_RESET_PIN -1
#define BOARD_DISPLAY_SPEED_HZ SPI_MASTER_FREQ_80M;
#define BOARD_DISPLAY_HOST BOARD_SPI
#define BOARD_DISPLAY_WIDTH 240
#define BOARD_DISPLAY_HEIGHT 320
#define BOARD_DISPLAY_OFFSETX 0
#define BOARD_DISPLAY_OFFSETY 0

#endif /* end of include guard: _BOARD_H_ */
