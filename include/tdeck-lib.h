#ifndef _TDECK_LIB_H_
#define _TDECK_LIB_H_

#include <board.h>
#include <battery.h>
#include <display.h>
#include <keyboard.h>
#include <trackball.h>
#include <speaker.h>
#include <sdcard.h>

esp_err_t td_board_init(td_board_t **Board);

#endif
/* end of include guard:  _TDECK_LIB_H_ */
