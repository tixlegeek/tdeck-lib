#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "esp_log.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <board.h>
#include <string.h>

// HOST/BUS --> DEVICES

typedef struct td_display_t {
  bool initialized;
  spi_device_handle_t dev;
  uint16_t _width;
	uint16_t _height;
	uint16_t _offsetx;
	uint16_t _offsety;
	uint16_t _font_direction;
	uint16_t _font_fill;
	uint16_t _font_fill_color;
	uint16_t _font_underline;
	uint16_t _font_underline_color;
	int16_t _dc;
	int16_t _bl;
	bool _use_frame_buffer;
	uint16_t *_frame_buffer;
  spi_host_device_t host;
} td_display_t;

esp_err_t td_display_init(void *ctx);
#endif /* end of include guard: _DISPLAY_H_ */
