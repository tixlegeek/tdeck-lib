#ifndef MAIN_ST7789_H_
#define MAIN_ST7789_H_
//https://github.com/nopnop2002/esp-idf-st7789
#include <driver/spi_master.h>
#include <board.h>
#include <display.h>

#include "fontx.h"
#include "esp_err.h"
#define rgb565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
#define hex565(hex) \
    ((((hex >> 16) & 0xF8) << 8) | \
     (((hex >> 8) & 0xFC) << 3) | \
     ((hex & 0xF8) >> 3))

#define RED rgb565(255, 0, 0)       // 0xf800
#define GREEN rgb565(0, 255, 0)     // 0x07e0
#define BLUE rgb565(0, 0, 255)      // 0x001f
#define BLACK rgb565(0, 0, 0)       // 0x0000
#define WHITE rgb565(255, 255, 255) // 0xffff
#define GRAY rgb565(128, 128, 128)  // 0x8410
#define YELLOW rgb565(255, 255, 0)  // 0xFFE0
#define CYAN rgb565(0, 156, 209)    // 0x04FA
#define PURPLE rgb565(128, 0, 128)  // 0x8010

typedef enum { DIRECTION0, DIRECTION90, DIRECTION180, DIRECTION270 } DIRECTION;

typedef enum {
  SCROLL_RIGHT = 1,
  SCROLL_LEFT = 2,
  SCROLL_DOWN = 3,
  SCROLL_UP = 4,
} SCROLL_TYPE_t;

void spi_clock_speed(int speed);
bool td_display_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data,
                           size_t DataLength);
bool td_display_write_command(td_board_t *Board, uint8_t cmd);
bool td_display_write_data_byte(td_board_t *Board, uint8_t data);
bool td_display_write_data_word(td_board_t *Board, uint16_t data);
bool td_display_write_addr(td_board_t *Board, uint16_t addr1, uint16_t addr2);
bool td_display_write_color(td_board_t *Board, uint16_t color, uint16_t size);
bool td_display_write_colors(td_board_t *Board, uint16_t *colors, uint16_t size);

void delayMS(int ms);
void lcdInit(td_board_t *Board, int width, int height, int offsetx, int offsety);
void lcdDrawPixel(td_board_t *Board, uint16_t x, uint16_t y, uint16_t color);
void lcdDrawMultiPixels(td_board_t *Board, uint16_t x, uint16_t y, uint16_t size,
                        uint16_t *colors);
void lcdDrawFillRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                     uint16_t y2, uint16_t color);
void lcdDrawFillSquare(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t size,
                       uint16_t color);
void lcdDisplayOff(td_board_t *Board);
void lcdDisplayOn(td_board_t *Board);
void lcdFillScreen(td_board_t *Board, uint16_t color);
void lcdDrawLine(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                 uint16_t y2, uint16_t color);
void lcdDrawRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                 uint16_t y2, uint16_t color);
void lcdDrawRectAngle(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t w,
                      uint16_t h, uint16_t angle, uint16_t color);
void lcdDrawTriangle(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t w,
                     uint16_t h, uint16_t angle, uint16_t color);
void lcdDrawRegularPolygon(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t n,
                           uint16_t r, uint16_t angle, uint16_t color);
void lcdDrawCircle(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                   uint16_t color);
void lcdDrawFillCircle(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                       uint16_t color);
void lcdDrawRoundRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                      uint16_t y2, uint16_t r, uint16_t color);
void lcdDrawArrow(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t x1,
                  uint16_t y1, uint16_t w, uint16_t color);
void lcdDrawFillArrow(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t x1,
                      uint16_t y1, uint16_t w, uint16_t color);
int lcdDrawChar(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
                uint8_t ascii, uint16_t color);
int lcdDrawString(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
                  uint8_t *ascii, uint16_t color);
int lcdDrawCode(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
                uint8_t code, uint16_t color);
// int lcdDrawUTF8Char(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
// uint8_t *utf8, uint16_t color); int lcdDrawUTF8String(td_board_t *Board,
// FontxFile *fx, uint16_t x, uint16_t y, unsigned char *utfs, uint16_t color);
void lcdSetFontDirection(td_board_t *Board, uint16_t);
void lcdSetFontFill(td_board_t *Board, uint16_t color);
void lcdUnsetFontFill(td_board_t *Board);
void lcdSetFontUnderLine(td_board_t *Board, uint16_t color);
void lcdUnsetFontUnderLine(td_board_t *Board);
void lcdBacklightOff(td_board_t *Board);
void lcdBacklightOn(td_board_t *Board);
void lcdInversionOff(td_board_t *Board);
void lcdInversionOn(td_board_t *Board);
void lcdWrapArround(td_board_t *Board, SCROLL_TYPE_t scroll, int start, int end);
void lcdInversionArea(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                      uint16_t y2, uint16_t *save);
void lcdGetRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                uint16_t y2, uint16_t *save);
void lcdSetRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                uint16_t y2, uint16_t *save);
void lcdSetCursor(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                  uint16_t color, uint16_t *save);
void lcdResetCursor(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                    uint16_t color, uint16_t *save);
void lcdDrawFinish(td_board_t *Board);
#endif /* MAIN_ST7789_H_ */
