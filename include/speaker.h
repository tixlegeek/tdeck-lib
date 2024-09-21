#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include <freertos/FreeRTOS.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#define EXAMPLE_BUFF_SIZE               2048
#define SAMPLE_RATE 16000
typedef struct td_speaker_t {
  i2s_chan_handle_t dev;
} td_speaker_t;

esp_err_t td_speaker_init(void *ctx);

#endif /* end of include guard: _SPEAKER_H_ */
