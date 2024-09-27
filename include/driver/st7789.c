//https://github.com/nopnop2002/esp-idf-st7789
#include "st7789.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#define TAG "ST7789"
#define _DEBUG_ 0

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;

void spi_clock_speed(int speed) {
  ESP_LOGI(TAG, "SPI clock speed=%d MHz", speed / 1000000);
}

bool td_display_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data,
                           size_t DataLength) {
  spi_transaction_t SPITransaction;
  esp_err_t ret;

  if (DataLength > 0) {
    memset(&SPITransaction, 0, sizeof(spi_transaction_t));
    SPITransaction.length = DataLength * 8;
    SPITransaction.tx_buffer = Data;
#if 1
    ret = spi_device_transmit(SPIHandle, &SPITransaction);
#else
    ret = spi_device_polling_transmit(SPIHandle, &SPITransaction);
#endif
    assert(ret == ESP_OK);
  }

  return true;
}

bool td_display_write_command(td_board_t *Board, uint8_t cmd) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte = 0;
  Byte = cmd;
  gpio_set_level(Display->_dc, SPI_Command_Mode);
  return td_display_write_byte(Display->dev, &Byte, 1);
}

bool td_display_write_data_byte(td_board_t *Board, uint8_t data) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte = 0;
  Byte = data;
  gpio_set_level(Display->_dc, SPI_Data_Mode);
  return td_display_write_byte(Display->dev, &Byte, 1);
}

bool td_display_write_data_word(td_board_t *Board, uint16_t data) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte[2];
  Byte[0] = (data >> 8) & 0xFF;
  Byte[1] = data & 0xFF;
  gpio_set_level(Display->_dc, SPI_Data_Mode);
  return td_display_write_byte(Display->dev, Byte, 2);
}

bool td_display_write_addr(td_board_t *Board, uint16_t addr1, uint16_t addr2) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte[4];
  Byte[0] = (addr1 >> 8) & 0xFF;
  Byte[1] = addr1 & 0xFF;
  Byte[2] = (addr2 >> 8) & 0xFF;
  Byte[3] = addr2 & 0xFF;
  gpio_set_level(Display->_dc, SPI_Data_Mode);
  return td_display_write_byte(Display->dev, Byte, 4);
}

bool td_display_write_color(td_board_t *Board, uint16_t color, uint16_t size) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte[1024];
  int index = 0;
  for (int i = 0; i < size; i++) {
    Byte[index++] = (color >> 8) & 0xFF;
    Byte[index++] = color & 0xFF;
  }
  gpio_set_level(Display->_dc, SPI_Data_Mode);
  return td_display_write_byte(Display->dev, Byte, size * 2);
}

// Add 202001
bool td_display_write_colors(td_board_t *Board, uint16_t *colors, uint16_t size) {
  assert(Board);
  td_display_t *Display = Board->Display;
  static uint8_t Byte[1024];
  int index = 0;
  for (int i = 0; i < size; i++) {
    Byte[index++] = (colors[i] >> 8) & 0xFF;
    Byte[index++] = colors[i] & 0xFF;
  }
  gpio_set_level(Display->_dc, SPI_Data_Mode);
  return td_display_write_byte(Display->dev, Byte, size * 2);
}

void delayMS(int ms) {
  int _ms = ms + (portTICK_PERIOD_MS - 1);
  TickType_t xTicksToDelay = _ms / portTICK_PERIOD_MS;
  ESP_LOGD(TAG,
           "ms=%d _ms=%d portTICK_PERIOD_MS=%" PRIu32 " xTicksToDelay=%" PRIu32,
           ms, _ms, portTICK_PERIOD_MS, xTicksToDelay);
  vTaskDelay(xTicksToDelay);
}
#define ST7789_MADCTL_BGR	BIT(3)
#define ST7789_MADCTL_MV	BIT(5)
#define ST7789_MADCTL_MX	BIT(6)
#define ST7789_MADCTL_MY	BIT(7)
void lcdInit(td_board_t *Board, int width, int height, int offsetx, int offsety) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_width = width;
  Display->_height = height;
  Display->_offsetx = offsetx;
  Display->_offsety = offsety;
  Display->_font_direction = DIRECTION0;
  Display->_font_fill = false;
  Display->_font_underline = false;

  td_display_write_command(Board, 0x01); // Software Reset
  delayMS(150);

  td_display_write_command(Board, 0x11); // Sleep Out
  delayMS(255);

  td_display_write_command(Board, 0x3A); // Interface Pixel Format
  td_display_write_data_byte(Board, 0x55);
  delayMS(10);

  td_display_write_command(Board, 0x36); // Memory Data Access Control
  td_display_write_data_byte(Board, ST7789_MADCTL_MX | ST7789_MADCTL_MV);

  td_display_write_command(Board, 0x2A); // Column Address Set
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0xF0);

  td_display_write_command(Board, 0x2B); // Row Address Set
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0x00);
  td_display_write_data_byte(Board, 0xF0);

  td_display_write_command(Board, 0x21); // Display Inversion On
  delayMS(10);

  td_display_write_command(Board, 0x13); // Normal Display Mode On
  delayMS(10);

  td_display_write_command(Board, 0x29); // Display ON
  delayMS(255);

  if (Display->_bl >= 0) {
    gpio_set_level(Display->_bl, 1);
  }

  Display->_frame_buffer =
      heap_caps_malloc(sizeof(uint16_t) * width * height, MALLOC_CAP_DMA);
  if (Display->_frame_buffer == NULL) {
    ESP_LOGE(TAG, "heap_caps_malloc fail");
  } else {
    ESP_LOGI(TAG, "heap_caps_malloc success");
    Display->_use_frame_buffer = false;
  }
}

// Draw pixel
// x:X coordinate
// y:Y coordinate
// color:color
void lcdDrawPixel(td_board_t *Board, uint16_t x, uint16_t y, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x >= Display->_width)
    return;
  if (y >= Display->_height)
    return;

  if (Display->_use_frame_buffer) {
    Display->_frame_buffer[y * Display->_width + x] = color;
  } else {
    uint16_t _x = x + Display->_offsetx;
    uint16_t _y = y + Display->_offsety;

    td_display_write_command(Board, 0x2A); // set column(x) address
    td_display_write_addr(Board, _x, _x);
    td_display_write_command(Board, 0x2B); // set Page(y) address
    td_display_write_addr(Board, _y, _y);
    td_display_write_command(Board, 0x2C); // Memory Write
    // td_display_write_data_word(Board, color);
    td_display_write_colors(Board, &color, 1);
  }
}

// Draw multi pixel
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
void lcdDrawMultiPixels(td_board_t *Board, uint16_t x, uint16_t y, uint16_t size,
                        uint16_t *colors) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x + size > Display->_width)
    return;
  if (y >= Display->_height)
    return;

  if (Display->_use_frame_buffer) {
    uint16_t _x1 = x;
    uint16_t _x2 = _x1 + (size - 1);
    uint16_t _y1 = y;
    uint16_t _y2 = _y1;
    int16_t index = 0;
    for (int16_t j = _y1; j <= _y2; j++) {
      for (int16_t i = _x1; i <= _x2; i++) {
        Display->_frame_buffer[j * Display->_width + i] = colors[index++];
      }
    }
  } else {
    uint16_t _x1 = x + Display->_offsetx;
    uint16_t _x2 = _x1 + (size - 1);
    uint16_t _y1 = y + Display->_offsety;
    uint16_t _y2 = _y1;

    td_display_write_command(Board, 0x2A); // set column(x) address
    td_display_write_addr(Board, _x1, _x2);
    td_display_write_command(Board, 0x2B); // set Page(y) address
    td_display_write_addr(Board, _y1, _y2);
    td_display_write_command(Board, 0x2C); // Memory Write
    td_display_write_colors(Board, colors, size);
  }
}

// Draw rectangle of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color
void lcdDrawFillRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                     uint16_t y2, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x1 >= Display->_width)
    return;
  if (x2 >= Display->_width)
    x2 = Display->_width - 1;
  if (y1 >= Display->_height)
    return;
  if (y2 >= Display->_height)
    y2 = Display->_height - 1;

  ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", Display->_offsetx,
           Display->_offsety);
  if (Display->_frame_buffer == NULL) {
    ESP_LOGE(TAG, "Framebuffer Error");
    return;
  }
  if (Display->_use_frame_buffer) {
    for (int16_t j = y1; j <= y2; j++) {
      for (int16_t i = x1; i <= x2; i++) {
        Display->_frame_buffer[j * Display->_width + i] = color;
      }
    }
  } else {
    uint16_t _x1 = x1 + Display->_offsetx;
    uint16_t _x2 = x2 + Display->_offsetx;
    uint16_t _y1 = y1 + Display->_offsety;
    uint16_t _y2 = y2 + Display->_offsety;

    td_display_write_command(Board, 0x2A); // set column(x) address
    td_display_write_addr(Board, _x1, _x2);
    td_display_write_command(Board, 0x2B); // set Page(y) address
    td_display_write_addr(Board, _y1, _y2);
    td_display_write_command(Board, 0x2C); // Memory Write
    for (int i = _x1; i <= _x2; i++) {
      uint16_t size = _y2 - _y1 + 1;
      td_display_write_color(Board, color, size);
    }
  }
}

// Draw square of filling
// x0:Center X coordinate
// y0:Center Y coordinate
// size:Square size
// color:color
void lcdDrawFillSquare(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t size,
                       uint16_t color) {
  assert(Board);
  uint16_t x1 = x0 - size;
  uint16_t y1 = y0 - size;
  uint16_t x2 = x0 + size;
  uint16_t y2 = y0 + size;
  lcdDrawFillRect(Board, x1, y1, x2, y2, color);
}

// Display OFF
void lcdDisplayOff(td_board_t *Board) {
  assert(Board);
  td_display_write_command(Board, 0x28); // Display off
}

// Display ON
void lcdDisplayOn(td_board_t *Board) {
  assert(Board);
  td_display_write_command(Board, 0x29); // Display on
}

// Fill screen
// color:color
void lcdFillScreen(td_board_t *Board, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  lcdDrawFillRect(Board, 0, 0, Display->_width - 1, Display->_height - 1,
                  color);
}

// Draw line
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// color:color
void lcdDrawLine(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                 uint16_t y2, uint16_t color) {
  assert(Board);
  int i;
  int dx, dy;
  int sx, sy;
  int E;

  /* distance between two points */
  dx = (x2 > x1) ? x2 - x1 : x1 - x2;
  dy = (y2 > y1) ? y2 - y1 : y1 - y2;

  /* direction of two point */
  sx = (x2 > x1) ? 1 : -1;
  sy = (y2 > y1) ? 1 : -1;

  /* inclination < 1 */
  if (dx > dy) {
    E = -dx;
    for (i = 0; i <= dx; i++) {
      lcdDrawPixel(Board, x1, y1, color);
      x1 += sx;
      E += 2 * dy;
      if (E >= 0) {
        y1 += sy;
        E -= 2 * dx;
      }
    }

    /* inclination >= 1 */
  } else {
    E = -dy;
    for (i = 0; i <= dy; i++) {
      lcdDrawPixel(Board, x1, y1, color);
      y1 += sy;
      E += 2 * dx;
      if (E >= 0) {
        x1 += sx;
        E -= 2 * dy;
      }
    }
  }
}

// Draw rectangle
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// color:color
void lcdDrawRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                 uint16_t y2, uint16_t color) {
  assert(Board);
  lcdDrawLine(Board, x1, y1, x2, y1, color);
  lcdDrawLine(Board, x2, y1, x2, y2, color);
  lcdDrawLine(Board, x2, y2, x1, y2, color);
  lcdDrawLine(Board, x1, y2, x1, y1, color);
}

// Draw rectangle with angle
// xc:Center X coordinate
// yc:Center Y coordinate
// w:Width of rectangle
// h:Height of rectangle
// angle:Angle of rectangle
// color:color

// When the origin is (0, 0), the point (x1, y1) after rotating the point (x, y)
// by the angle is obtained by the following calculation.
//  x1 = x * cos(angle) - y * sin(angle)
//  y1 = x * sin(angle) + y * cos(angle)
void lcdDrawRectAngle(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t w,
                      uint16_t h, uint16_t angle, uint16_t color) {
  assert(Board);
  double xd, yd, rd;
  int x1, y1;
  int x2, y2;
  int x3, y3;
  int x4, y4;
  rd = -angle * M_PI / 180.0;
  xd = 0.0 - w / 2;
  yd = h / 2;
  x1 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y1 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  yd = 0.0 - yd;
  x2 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y2 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  xd = w / 2;
  yd = h / 2;
  x3 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y3 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  yd = 0.0 - yd;
  x4 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y4 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  lcdDrawLine(Board, x1, y1, x2, y2, color);
  lcdDrawLine(Board, x1, y1, x3, y3, color);
  lcdDrawLine(Board, x2, y2, x4, y4, color);
  lcdDrawLine(Board, x3, y3, x4, y4, color);
}

// Draw triangle
// xc:Center X coordinate
// yc:Center Y coordinate
// w:Width of triangle
// h:Height of triangle
// angle:Angle of triangle
// color:color

// When the origin is (0, 0), the point (x1, y1) after rotating the point (x, y)
// by the angle is obtained by the following calculation.
//  x1 = x * cos(angle) - y * sin(angle)
//  y1 = x * sin(angle) + y * cos(angle)
void lcdDrawTriangle(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t w,
                     uint16_t h, uint16_t angle, uint16_t color) {
  assert(Board);
  double xd, yd, rd;
  int x1, y1;
  int x2, y2;
  int x3, y3;
  rd = -angle * M_PI / 180.0;
  xd = 0.0;
  yd = h / 2;
  x1 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y1 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  xd = w / 2;
  yd = 0.0 - yd;
  x2 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y2 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  xd = 0.0 - w / 2;
  x3 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
  y3 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

  lcdDrawLine(Board, x1, y1, x2, y2, color);
  lcdDrawLine(Board, x1, y1, x3, y3, color);
  lcdDrawLine(Board, x2, y2, x3, y3, color);
}

// Draw regular polygon
// xc:Center X coordinate
// yc:Center Y coordinate
// n:Number of slides
// r:radius
// angle:Angle of regular polygon
// color:color
void lcdDrawRegularPolygon(td_board_t *Board, uint16_t xc, uint16_t yc, uint16_t n,
                           uint16_t r, uint16_t angle, uint16_t color) {
  assert(Board);
  double xd, yd, rd;
  int x1, y1;
  int x2, y2;
  int i;

  rd = -angle * M_PI / 180.0;
  for (i = 0; i < n; i++) {
    xd = r * cos(2 * M_PI * i / n);
    yd = r * sin(2 * M_PI * i / n);
    x1 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
    y1 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

    xd = r * cos(2 * M_PI * (i + 1) / n);
    yd = r * sin(2 * M_PI * (i + 1) / n);
    x2 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
    y2 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

    lcdDrawLine(Board, x1, y1, x2, y2, color);
  }
}

// Draw circle
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawCircle(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                   uint16_t color) {
  assert(Board);
  int x;
  int y;
  int err;
  int old_err;

  x = 0;
  y = -r;
  err = 2 - 2 * r;
  do {
    lcdDrawPixel(Board, x0 - x, y0 + y, color);
    lcdDrawPixel(Board, x0 - y, y0 - x, color);
    lcdDrawPixel(Board, x0 + x, y0 - y, color);
    lcdDrawPixel(Board, x0 + y, y0 + x, color);
    if ((old_err = err) <= x)
      err += ++x * 2 + 1;
    if (old_err > y || err > x)
      err += ++y * 2 + 1;
  } while (y < 0);
}

// Draw circle of filling
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawFillCircle(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                       uint16_t color) {
  assert(Board);
  int x;
  int y;
  int err;
  int old_err;
  int ChangeX;

  x = 0;
  y = -r;
  err = 2 - 2 * r;
  ChangeX = 1;
  do {
    if (ChangeX) {
      lcdDrawLine(Board, x0 - x, y0 - y, x0 - x, y0 + y, color);
      lcdDrawLine(Board, x0 + x, y0 - y, x0 + x, y0 + y, color);
    } // endif
    ChangeX = (old_err = err) <= x;
    if (ChangeX)
      err += ++x * 2 + 1;
    if (old_err > y || err > x)
      err += ++y * 2 + 1;
  } while (y <= 0);
}

// Draw rectangle with round corner
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// r:radius
// color:color
void lcdDrawRoundRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                      uint16_t y2, uint16_t r, uint16_t color) {
  assert(Board);
  int x;
  int y;
  int err;
  int old_err;
  unsigned char temp;

  if (x1 > x2) {
    temp = x1;
    x1 = x2;
    x2 = temp;
  } // endif

  if (y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  } // endif

  ESP_LOGD(TAG, "x1=%d x2=%d delta=%d r=%d", x1, x2, x2 - x1, r);
  ESP_LOGD(TAG, "y1=%d y2=%d delta=%d r=%d", y1, y2, y2 - y1, r);
  if (x2 - x1 < r)
    return; // Add 20190517
  if (y2 - y1 < r)
    return; // Add 20190517

  x = 0;
  y = -r;
  err = 2 - 2 * r;

  do {
    if (x) {
      lcdDrawPixel(Board, x1 + r - x, y1 + r + y, color);
      lcdDrawPixel(Board, x2 - r + x, y1 + r + y, color);
      lcdDrawPixel(Board, x1 + r - x, y2 - r - y, color);
      lcdDrawPixel(Board, x2 - r + x, y2 - r - y, color);
    } // endif
    if ((old_err = err) <= x)
      err += ++x * 2 + 1;
    if (old_err > y || err > x)
      err += ++y * 2 + 1;
  } while (y < 0);

  ESP_LOGD(TAG, "x1+r=%d x2-r=%d", x1 + r, x2 - r);
  lcdDrawLine(Board, x1 + r, y1, x2 - r, y1, color);
  lcdDrawLine(Board, x1 + r, y2, x2 - r, y2, color);
  ESP_LOGD(TAG, "y1+r=%d y2-r=%d", y1 + r, y2 - r);
  lcdDrawLine(Board, x1, y1 + r, x1, y2 - r, color);
  lcdDrawLine(Board, x2, y1 + r, x2, y2 - r, color);
}

// Draw arrow
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// w:Width of the botom
// color:color
// Thanks http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html
void lcdDrawArrow(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t x1,
                  uint16_t y1, uint16_t w, uint16_t color) {
  assert(Board);
  double Vx = x1 - x0;
  double Vy = y1 - y0;
  double v = sqrt(Vx * Vx + Vy * Vy);
  //	 printf("v=%f\n",v);
  double Ux = Vx / v;
  double Uy = Vy / v;

  uint16_t L[2], R[2];
  L[0] = x1 - Uy * w - Ux * v;
  L[1] = y1 + Ux * w - Uy * v;
  R[0] = x1 + Uy * w - Ux * v;
  R[1] = y1 - Ux * w - Uy * v;
  // printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

  // lcdDrawLine(x0,y0,x1,y1,color);
  lcdDrawLine(Board, x1, y1, L[0], L[1], color);
  lcdDrawLine(Board, x1, y1, R[0], R[1], color);
  lcdDrawLine(Board, L[0], L[1], R[0], R[1], color);
}

// Draw arrow of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// w:Width of the botom
// color:color
void lcdDrawFillArrow(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t x1,
                      uint16_t y1, uint16_t w, uint16_t color) {
  assert(Board);
  double Vx = x1 - x0;
  double Vy = y1 - y0;
  double v = sqrt(Vx * Vx + Vy * Vy);
  // printf("v=%f\n",v);
  double Ux = Vx / v;
  double Uy = Vy / v;

  uint16_t L[2], R[2];
  L[0] = x1 - Uy * w - Ux * v;
  L[1] = y1 + Ux * w - Uy * v;
  R[0] = x1 + Uy * w - Ux * v;
  R[1] = y1 - Ux * w - Uy * v;
  // printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

  lcdDrawLine(Board, x0, y0, x1, y1, color);
  lcdDrawLine(Board, x1, y1, L[0], L[1], color);
  lcdDrawLine(Board, x1, y1, R[0], R[1], color);
  lcdDrawLine(Board, L[0], L[1], R[0], R[1], color);

  int ww;
  for (ww = w - 1; ww > 0; ww--) {
    L[0] = x1 - Uy * ww - Ux * v;
    L[1] = y1 + Ux * ww - Uy * v;
    R[0] = x1 + Uy * ww - Ux * v;
    R[1] = y1 - Ux * ww - Uy * v;
    // printf("Fill>L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);
    lcdDrawLine(Board, x1, y1, L[0], L[1], color);
    lcdDrawLine(Board, x1, y1, R[0], R[1], color);
  }
}

// Draw ASCII character
// x:X coordinate
// y:Y coordinate
// ascii: ascii code
// color:color
int lcdDrawChar(td_board_t *Board, FontxFile *fxs, uint16_t x, uint16_t y,
                uint8_t ascii, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  uint16_t xx, yy, bit, ofs;
  unsigned char fonts[128]; // font pattern
  unsigned char pw, ph;
  int h, w;
  uint16_t mask;
  bool rc;

  if (_DEBUG_)
    printf("_font_direction=%d\n", Display->_font_direction);
  rc = GetFontx(fxs, ascii, fonts, &pw, &ph);
  if (_DEBUG_)
    printf("GetFontx rc=%d pw=%d ph=%d\n", rc, pw, ph);
  if (!rc)
    return 0;

  int16_t xd1 = 0;
  int16_t yd1 = 0;
  int16_t xd2 = 0;
  int16_t yd2 = 0;
  uint16_t xss = 0;
  uint16_t yss = 0;
  int16_t xsd = 0;
  int16_t ysd = 0;
  int16_t next = 0;
  uint16_t x0 = 0;
  uint16_t x1 = 0;
  uint16_t y0 = 0;
  uint16_t y1 = 0;
  if (Display->_font_direction == 0) {
    xd1 = +1;
    yd1 = +1; //-1;
    xd2 = 0;
    yd2 = 0;
    xss = x;
    yss = y - (ph - 1);
    xsd = 1;
    ysd = 0;
    next = x + pw;

    x0 = x;
    y0 = y - (ph - 1);
    x1 = x + (pw - 1);
    y1 = y;
  } else if (Display->_font_direction == 2) {
    xd1 = -1;
    yd1 = -1; //+1;
    xd2 = 0;
    yd2 = 0;
    xss = x;
    yss = y + ph + 1;
    xsd = 1;
    ysd = 0;
    next = x - pw;

    x0 = x - (pw - 1);
    y0 = y;
    x1 = x;
    y1 = y + (ph - 1);
  } else if (Display->_font_direction == 1) {
    xd1 = 0;
    yd1 = 0;
    xd2 = -1;
    yd2 = +1; //-1;
    xss = x + ph;
    yss = y;
    xsd = 0;
    ysd = 1;
    next = y + pw; // y - pw;

    x0 = x;
    y0 = y;
    x1 = x + (ph - 1);
    y1 = y + (pw - 1);
  } else if (Display->_font_direction == 3) {
    xd1 = 0;
    yd1 = 0;
    xd2 = +1;
    yd2 = -1; //+1;
    xss = x - (ph - 1);
    yss = y;
    xsd = 0;
    ysd = 1;
    next = y - pw; // y + pw;

    x0 = x - (ph - 1);
    y0 = y - (pw - 1);
    x1 = x;
    y1 = y;
  }

  if (Display->_font_fill)
    lcdDrawFillRect(Board, x0, y0, x1, y1, Display->_font_fill_color);

  int bits;
  if (_DEBUG_)
    printf("xss=%d yss=%d\n", xss, yss);
  ofs = 0;
  yy = yss;
  xx = xss;
  for (h = 0; h < ph; h++) {
    if (xsd)
      xx = xss;
    if (ysd)
      yy = yss;
    // for(w=0;w<(pw/8);w++) {
    bits = pw;
    for (w = 0; w < ((pw + 4) / 8); w++) {
      mask = 0x80;
      for (bit = 0; bit < 8; bit++) {
        bits--;
        if (bits < 0)
          continue;
        // if(_DEBUG_)printf("xx=%d yy=%d mask=%02x
        // fonts[%d]=%02x\n",xx,yy,mask,ofs,fonts[ofs]);
        if (fonts[ofs] & mask) {
          lcdDrawPixel(Board, xx, yy, color);
        } else {
          // if (Display->_font_fill) lcdDrawPixel(Board, xx, yy,
          // Display->_font_fill_color);
        }
        if (h == (ph - 2) && Display->_font_underline)
          lcdDrawPixel(Board, xx, yy, Display->_font_underline_color);
        if (h == (ph - 1) && Display->_font_underline)
          lcdDrawPixel(Board, xx, yy, Display->_font_underline_color);
        xx = xx + xd1;
        yy = yy + yd2;
        mask = mask >> 1;
      }
      ofs++;
    }
    yy = yy + yd1;
    xx = xx + xd2;
  }

  if (next < 0)
    next = 0;
  return next;
}

int lcdDrawString(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
                  uint8_t *ascii, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  int length = strlen((char *)ascii);
  if (_DEBUG_)
    printf("lcdDrawString length=%d\n", length);
  for (int i = 0; i < length; i++) {
    if (_DEBUG_)
      printf("ascii[%d]=%x x=%d y=%d\n", i, ascii[i], x, y);
    if (Display->_font_direction == 0)
      x = lcdDrawChar(Board, fx, x, y, ascii[i], color);
    if (Display->_font_direction == 1)
      y = lcdDrawChar(Board, fx, x, y, ascii[i], color);
    if (Display->_font_direction == 2)
      x = lcdDrawChar(Board, fx, x, y, ascii[i], color);
    if (Display->_font_direction == 3)
      y = lcdDrawChar(Board, fx, x, y, ascii[i], color);
  }
  if (Display->_font_direction == 0)
    return x;
  if (Display->_font_direction == 2)
    return x;
  if (Display->_font_direction == 1)
    return y;
  if (Display->_font_direction == 3)
    return y;
  return 0;
}

// Draw Non-Alphanumeric character
// x:X coordinate
// y:Y coordinate
// code:character code
// color:color
int lcdDrawCode(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y,
                uint8_t code, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (_DEBUG_)
    printf("code=%x x=%d y=%d\n", code, x, y);
  if (Display->_font_direction == 0)
    x = lcdDrawChar(Board, fx, x, y, code, color);
  if (Display->_font_direction == 1)
    y = lcdDrawChar(Board, fx, x, y, code, color);
  if (Display->_font_direction == 2)
    x = lcdDrawChar(Board, fx, x, y, code, color);
  if (Display->_font_direction == 3)
    y = lcdDrawChar(Board, fx, x, y, code, color);
  if (Display->_font_direction == 0)
    return x;
  if (Display->_font_direction == 2)
    return x;
  if (Display->_font_direction == 1)
    return y;
  if (Display->_font_direction == 3)
    return y;
  return 0;
}

#if 0
// Draw UTF8 character
// x:X coordinate
// y:Y coordinate
// utf8:UTF8 code
// color:color
int lcdDrawUTF8Char(td_board_t *Board, FontxFile *fx, uint16_t x,uint16_t y,uint8_t *utf8,uint16_t color) {
	assert(Board);
	uint16_t sjis[1];
	sjis[0] = UTF2SJIS(utf8);
	if(_DEBUG_)printf("sjis=%04x\n",sjis[0]);
	return lcdDrawSJISChar(Board, fx, x, y, sjis[0], color);
}

// Draw UTF8 string
// x:X coordinate
// y:Y coordinate
// utfs:UTF8 string
// color:color
int lcdDrawUTF8String(td_board_t *Board, FontxFile *fx, uint16_t x, uint16_t y, unsigned char *utfs, uint16_t color) {
	assert(Board);
 td_display_t *Display = Board->Display;

	int i;
	int spos;
	uint16_t sjis[64];
	spos = String2SJIS(utfs, strlen((char *)utfs), sjis, 64);
	if(_DEBUG_)printf("spos=%d\n",spos);
	for(i=0;i<spos;i++) {
		if(_DEBUG_)printf("sjis[%d]=%x y=%d\n",i,sjis[i],y);
		if (Display->_font_direction == 0)
			x = lcdDrawSJISChar(Board, fx, x, y, sjis[i], color);
		if (Display->_font_direction == 1)
			y = lcdDrawSJISChar(Board, fx, x, y, sjis[i], color);
		if (Display->_font_direction == 2)
			x = lcdDrawSJISChar(Board, fx, x, y, sjis[i], color);
		if (Display->_font_direction == 3)
			y = lcdDrawSJISChar(Board, fx, x, y, sjis[i], color);
	}
	if (Display->_font_direction == 0) return x;
	if (Display->_font_direction == 2) return x;
	if (Display->_font_direction == 1) return y;
	if (Display->_font_direction == 3) return y;
	return 0;
}
#endif

// Set font direction
// dir:Direction
void lcdSetFontDirection(td_board_t *Board, uint16_t dir) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_font_direction = dir;
}

// Set font filling
// color:fill color
void lcdSetFontFill(td_board_t *Board, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_font_fill = true;
  Display->_font_fill_color = color;
}

// UnSet font filling
void lcdUnsetFontFill(td_board_t *Board) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_font_fill = false;
}

// Set font underline
// color:frame color
void lcdSetFontUnderLine(td_board_t *Board, uint16_t color) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_font_underline = true;
  Display->_font_underline_color = color;
}

// UnSet font underline
void lcdUnsetFontUnderLine(td_board_t *Board) {
  assert(Board);
  td_display_t *Display = Board->Display;
  Display->_font_underline = false;
}

// Backlight OFF
void lcdBacklightOff(td_board_t *Board) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (Display->_bl >= 0) {
    gpio_set_level(Display->_bl, 0);
  }
}

// Backlight ON
void lcdBacklightOn(td_board_t *Board) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (Display->_bl >= 0) {
    gpio_set_level(Display->_bl, 1);
  }
}

// Display Inversion Off
void lcdInversionOff(td_board_t *Board) {
  assert(Board);
  td_display_write_command(Board, 0x20); // Display Inversion Off
}

// Display Inversion On
void lcdInversionOn(td_board_t *Board) {
  assert(Board);
  td_display_write_command(Board, 0x21); // Display Inversion On
}

void lcdWrapArround(td_board_t *Board, SCROLL_TYPE_t scroll, int start, int end) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (Display->_use_frame_buffer == false)
    return;

  int _width = Display->_width;
  int _height = Display->_height;
  int32_t index1;
  int32_t index2;

  if (scroll == SCROLL_RIGHT) {
    uint16_t wk[_width];
    for (int i = start; i < end; i++) {
      index1 = i * _width;
      memcpy((char *)wk, (char *)&Display->_frame_buffer[index1], _width * 2);
      index2 = index1 + _width - 1;
      Display->_frame_buffer[index1] = Display->_frame_buffer[index2];
      memcpy((char *)&Display->_frame_buffer[index1 + 1], (char *)&wk[0],
             (_width - 1) * 2);
    }
  } else if (scroll == SCROLL_LEFT) {
    uint16_t wk[_width];
    for (int i = start; i < end; i++) {
      index1 = i * _width;
      memcpy((char *)wk, (char *)&Display->_frame_buffer[index1], _width * 2);
      index2 = index1 + _width - 1;
      Display->_frame_buffer[index2] = Display->_frame_buffer[index1];
      memcpy((char *)&Display->_frame_buffer[index1], (char *)&wk[1],
             (_width - 1) * 2);
    }
  } else if (scroll == SCROLL_UP) {
    uint16_t wk;
    for (int i = start; i <= end; i++) {
      wk = Display->_frame_buffer[i];
      for (int j = 0; j < _height - 1; j++) {
        index1 = j * _width + i;
        index2 = (j + 1) * _width + i;
        Display->_frame_buffer[index1] = Display->_frame_buffer[index2];
      }
      index2 = (_height - 1) * _width + i;
      Display->_frame_buffer[index2] = wk;
    }
  } else if (scroll == SCROLL_DOWN) {
    uint16_t wk;
    for (int i = start; i <= end; i++) {
      index2 = (_height - 1) * _width + i;
      wk = Display->_frame_buffer[index2];
      for (int j = _height - 2; j >= 0; j--) {
        index1 = j * _width + i;
        index2 = (j + 1) * _width + i;
        Display->_frame_buffer[index2] = Display->_frame_buffer[index1];
      }
      Display->_frame_buffer[i] = wk;
    }
  }
}

// Invert a rectangular area
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// save:Save buffer
void lcdInversionArea(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                      uint16_t y2, uint16_t *save) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x1 >= Display->_width)
    return;
  if (x2 >= Display->_width)
    x2 = Display->_width - 1;
  if (y1 >= Display->_height)
    return;
  if (y2 >= Display->_height)
    y2 = Display->_height - 1;

  int index = 0;
  ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", Display->_offsetx,
           Display->_offsety);
  if (Display->_use_frame_buffer) {
    for (int16_t j = y1; j <= y2; j++) {
      for (int16_t i = x1; i <= x2; i++) {
        if (save)
          save[index++] = Display->_frame_buffer[j * Display->_width + i];
        Display->_frame_buffer[j * Display->_width + i] =
            ~Display->_frame_buffer[j * Display->_width + i];
      }
    }
  } else {
    ESP_LOGW(TAG, "To use this feature, enable the FrameBuffer option.");
  }
}

// Get rectangle area from frame buffer
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// save:Save buffer
void lcdGetRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                uint16_t y2, uint16_t *save) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x1 >= Display->_width)
    return;
  if (x2 >= Display->_width)
    x2 = Display->_width - 1;
  if (y1 >= Display->_height)
    return;
  if (y2 >= Display->_height)
    y2 = Display->_height - 1;

  int index = 0;
  ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", Display->_offsetx,
           Display->_offsety);
  if (Display->_use_frame_buffer) {
    for (int16_t j = y1; j <= y2; j++) {
      for (int16_t i = x1; i <= x2; i++) {
        save[index++] = Display->_frame_buffer[j * Display->_width + i];
      }
    }
  } else {
    ESP_LOGW(TAG, "Disable frame buffer");
  }
}

// Set rectangle area to frame buffer
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// save:Save buffer
void lcdSetRect(td_board_t *Board, uint16_t x1, uint16_t y1, uint16_t x2,
                uint16_t y2, uint16_t *save) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (x1 >= Display->_width)
    return;
  if (x2 >= Display->_width)
    x2 = Display->_width - 1;
  if (y1 >= Display->_height)
    return;
  if (y2 >= Display->_height)
    y2 = Display->_height - 1;

  int index = 0;
  ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", Display->_offsetx,
           Display->_offsety);
  if (Display->_use_frame_buffer) {
    for (int16_t j = y1; j <= y2; j++) {
      for (int16_t i = x1; i <= x2; i++) {
        Display->_frame_buffer[j * Display->_width + i] = save[index++];
      }
    }
  } else {
    ESP_LOGW(TAG, "Disable frame buffer");
  }
}

// Draw circle as cursor
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdSetCursor(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                  uint16_t color, uint16_t *save) {
  assert(Board);
  lcdGetRect(Board, x0 - r, y0 - r, x0 + r, y0 + r, save);
  lcdDrawCircle(Board, x0, y0, r, color);
}

void lcdResetCursor(td_board_t *Board, uint16_t x0, uint16_t y0, uint16_t r,
                    uint16_t color, uint16_t *save) {
  assert(Board);
  lcdSetRect(Board, x0 - r, y0 - r, x0 + r, y0 + r, save);
  // lcdDrawCircle(Board, x0, y0, r, color);
}

// Draw Frame Buffer
void lcdDrawFinish(td_board_t *Board) {
  assert(Board);
  td_display_t *Display = Board->Display;
  if (Display->_use_frame_buffer == false)
    return;

  td_display_write_command(Board, 0x2A); // set column(x) address
  td_display_write_addr(Board, Display->_offsetx,
                        Display->_offsetx + Display->_width - 1);
  td_display_write_command(Board, 0x2B); // set Page(y) address
  td_display_write_addr(Board, Display->_offsety,
                        Display->_offsety + Display->_height - 1);
  td_display_write_command(Board, 0x2C); // Memory Write

  // uint16_t size = Display->_width*Display->_height;
  uint32_t size = Display->_width * Display->_height;
  uint16_t *image = Display->_frame_buffer;
  while (size > 0) {
    // 1024 bytes per time.
    uint16_t bs = (size > 1024) ? 1024 : size;
    td_display_write_colors(Board, image, bs);
    size -= bs;
    image += bs;
  }
  return;
}
