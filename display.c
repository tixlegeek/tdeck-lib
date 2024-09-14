
#include <board.h>
#include <display.h>
#include <driver/spi_master.h>
#include <st7789.h>

const static char *TAG = "DISPLAY";

esp_err_t td_display_init(void *ctx) {
  td_board_t *Board = (td_board_t*)ctx;
  td_display_t *Display = &Board->Display;

  esp_err_t ret;

  if (BOARD_DISPLAY_CS_PIN >= 0) {
    // gpio_pad_select_gpio( BOARD_DISPLAY_CS_PIN );
    gpio_reset_pin(BOARD_DISPLAY_CS_PIN);
    gpio_set_direction(BOARD_DISPLAY_CS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_DISPLAY_CS_PIN, 0);
  }

  // gpio_pad_select_gpio( BOARD_DISPLAY_DC_PIN );
  gpio_reset_pin(BOARD_DISPLAY_DC_PIN);
  gpio_set_direction(BOARD_DISPLAY_DC_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(BOARD_DISPLAY_DC_PIN, 0);

  if (BOARD_DISPLAY_RESET_PIN >= 0) {
    // gpio_pad_select_gpio( BOARD_DISPLAY_RESET_PIN );
    gpio_reset_pin(BOARD_DISPLAY_RESET_PIN);
    gpio_set_direction(BOARD_DISPLAY_RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_DISPLAY_RESET_PIN, 1);
    delayMS(100);
    gpio_set_level(BOARD_DISPLAY_RESET_PIN, 0);
    delayMS(100);
    gpio_set_level(BOARD_DISPLAY_RESET_PIN, 1);
    delayMS(100);
  }

  ESP_LOGI(TAG, "BOARD_DISPLAY_BL_PIN=%d", BOARD_DISPLAY_BL_PIN);
  if (BOARD_DISPLAY_BL_PIN >= 0) {
    // gpio_pad_select_gpio(BOARD_DISPLAY_BL_PIN);
    gpio_reset_pin(BOARD_DISPLAY_BL_PIN);
    gpio_set_direction(BOARD_DISPLAY_BL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_DISPLAY_BL_PIN, 0);
  }

  assert(ret == ESP_OK);

  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = BOARD_DISPLAY_SPEED_HZ;
  devcfg.queue_size = 7;
  // devcfg.mode = 2;
  devcfg.mode = 3;
  devcfg.flags = SPI_DEVICE_NO_DUMMY;

  if (BOARD_DISPLAY_CS_PIN >= 0) {
    devcfg.spics_io_num = BOARD_DISPLAY_CS_PIN;
  } else {
    devcfg.spics_io_num = -1;
  }

  spi_device_handle_t handle;
  ret = spi_bus_add_device(BOARD_SPI, &devcfg, &handle);
  if (ret != ESP_OK) {
    return ret;
  }
  ESP_LOGD(TAG, "spi_bus_add_device=%d", ret);
  assert(ret == ESP_OK);
  Display->_dc = BOARD_DISPLAY_DC_PIN;
  Display->_bl = BOARD_DISPLAY_BL_PIN;
  Display->dev = handle;
  lcdInit(Board, BOARD_DISPLAY_WIDTH, BOARD_DISPLAY_HEIGHT,
          BOARD_DISPLAY_OFFSETX, BOARD_DISPLAY_OFFSETY);
  lcdBacklightOn(Board);
  lcdSetFontDirection(Board, 1);
  lcdFillScreen(Board, CYAN);
  lcdDrawFinish(Board);
  return ESP_OK;
}
