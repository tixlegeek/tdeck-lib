#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_

#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

typedef enum {
  EVT_TRACKBALL_UP = (1 << 0),
  EVT_TRACKBALL_DOWN = (1 << 1),
  EVT_TRACKBALL_LEFT = (1 << 2),
  EVT_TRACKBALL_RIGHT = (1 << 3),
} td_trackball_evt_t;

typedef struct td_trackball_t {
  void *test;
} td_trackball_t;

static void IRAM_ATTR td_trackball_int(void *_evt);
char *td_trackball_evt2str(td_trackball_evt_t evt);
void td_trackball_task(void *ctx);
esp_err_t td_trackball_init(void *ctx);

#endif /* end of include guard: _TRACKBALL_H_ */
