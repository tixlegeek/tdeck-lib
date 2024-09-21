#include <board.h>
#include <trackball.h>

const static char *TAG = "TRACKBALL";
QueueHandle_t td_trackball_queue = NULL;

static void IRAM_ATTR td_trackball_int(void* _evt)
{
    td_trackball_evt_t evt = (td_trackball_evt_t)_evt;
    xQueueSendFromISR(td_trackball_queue, &evt, NULL);

}

esp_err_t td_trackball_init(void *ctx){
    td_board_t *Board = (td_board_t *)ctx;
    gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_ANYEDGE,//disable interrupt
      .mode = GPIO_MODE_INPUT,//set as output mode
      .pin_bit_mask = BOARD_TRACKBALL_PINSEL,//bit mask of the pins that you want to set,e.g.GPIO18/19
      .pull_down_en = 0,//disable pull-down mode
      .pull_up_en = 0,//disable pull-up mode
    };
    gpio_config(&io_conf);
    td_trackball_queue = xQueueCreate(10, sizeof(td_trackball_evt_t));

    gpio_isr_handler_add(BOARD_TRACKBALL_UP_PIN, td_trackball_int, (void*)EVT_TRACKBALL_UP);
    gpio_isr_handler_add(BOARD_TRACKBALL_DOWN_PIN, td_trackball_int, (void*)EVT_TRACKBALL_DOWN);
    gpio_isr_handler_add(BOARD_TRACKBALL_LEFT_PIN, td_trackball_int, (void*)EVT_TRACKBALL_LEFT);
    gpio_isr_handler_add(BOARD_TRACKBALL_RIGHT_PIN, td_trackball_int, (void*)EVT_TRACKBALL_RIGHT);

    return ESP_OK;
}
