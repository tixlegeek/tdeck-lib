#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_

#include <freertos/FreeRTOS.h>
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define EVT_TRACKBALL_UP    (1 << 0)
#define EVT_TRACKBALL_DOWN  (1 << 1)
#define EVT_TRACKBALL_LEFT  (1 << 2)
#define EVT_TRACKBALL_RIGHT (1 << 3)

typedef struct td_trackball_t {
  void *test;
} td_trackball_t;

typedef void(*td_trackball_cb)(void*ctx, int dirx, int diry);

void td_trackball_set_cb(td_trackball_cb cb);
char *td_trackball_evt2str(uint8_t evt);
void td_trackball_task(void *ctx);
esp_err_t td_trackball_init(void *ctx);

#endif /* end of include guard: _TRACKBALL_H_ */
