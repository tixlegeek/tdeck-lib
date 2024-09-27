#include <board.h>
#include <trackball.h>

const static char *TAG = "TRACKBALL";
static EventGroupHandle_t td_trackball_evtgroup;

static char *td_trackball_evt_str[] = {
    "EVT_UP",
    "EVT_DOWN",
    "EVT_LEFT",
    "EVT_RIGHT",
};
td_trackball_cb trackball_cb;
void td_trackball_set_cb(td_trackball_cb cb) { trackball_cb = cb; }

static void IRAM_ATTR td_trackball_int(void *_evt) {
  uint8_t evt = (uint8_t)_evt;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xEventGroupSetBitsFromISR(td_trackball_evtgroup, evt,
                            &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR(); // Yield if a higher priority task needs to run
  }
}

char *td_trackball_evt2str(uint8_t evt) {
  UNUSED(evt);
  return td_trackball_evt_str[0];
}
#define MAX_INTERVAL 10
void td_trackball_task(void *ctx) {
  assert(ctx != NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_trackball_t *Trackball = Board->Trackball;
  int8_t dx = 0;
  int8_t dy = 0;
  TickType_t lastTick=0;
  TickType_t intervalTick=0;

  size_t speed = 0;
  size_t ramp = 0;

  if (Trackball == NULL) {
    vTaskDelete(NULL);
  }
  EventBits_t evtbits = 0;
  while (td_trackball_evtgroup == NULL) {
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1));
    lastTick = xTaskGetTickCount();
    evtbits = xEventGroupWaitBits(
        td_trackball_evtgroup,
        EVT_TRACKBALL_UP | EVT_TRACKBALL_DOWN | EVT_TRACKBALL_LEFT |
            EVT_TRACKBALL_RIGHT, // The bits to wait for
        pdFALSE,                 // Clear bits on exit
        pdFALSE,                 // Wait for both bits
        portMAX_DELAY            // Wait indefinitely
    );

    intervalTick = xTaskGetTickCount() - lastTick;

    if(intervalTick > MAX_INTERVAL){
      speed = 1;
    } else {
      if(speed < MAX_INTERVAL)
        speed++;
    }
    // Attente des données dans la queue
    if (evtbits & EVT_TRACKBALL_UP) {
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_UP);
      dy = -1;
    } else if (evtbits & EVT_TRACKBALL_DOWN) {
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_DOWN);
      dy = 1;
    } else {
      dy = 0;
    }

    if (evtbits & EVT_TRACKBALL_LEFT) {
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_LEFT);
      dx = -1;
    } else if (evtbits & EVT_TRACKBALL_RIGHT) {
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_RIGHT);
      dx = 1;
    } else {
      dx = 0;
    }

    if ((dx != 0) || (dy != 0)) {
      if (trackball_cb){
        trackball_cb(Board, dx*speed, dy*speed);
        printf("SPEED: %d \n", speed);
      }
    }
  }
  vTaskDelete(NULL);
}

esp_err_t td_trackball_init(void *ctx) {
  assert(ctx != NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_trackball_t *Trackball = Board->Trackball;

  if (Trackball != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  Trackball = malloc(sizeof(td_trackball_t));
  if (Trackball == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->Trackball = Trackball;

  gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_ANYEDGE,         // disable interrupt
      .mode = GPIO_MODE_INPUT,                // set as output mode
      .pin_bit_mask = BOARD_TRACKBALL_PINSEL, // bit mask of the pins that you
                                              // want to set,e.g.GPIO18/19
      .pull_down_en = 0,                      // disable pull-down mode
      .pull_up_en = 0,                        // disable pull-up mode
  };

  gpio_config(&io_conf);

  td_trackball_evtgroup = xEventGroupCreate();
  if (td_trackball_evtgroup == NULL) {
    ESP_LOGE(TAG, "Event group creation failed!");
    return ESP_FAIL;
  }
  gpio_isr_handler_add(BOARD_TRACKBALL_UP_PIN, td_trackball_int,
                       (void *)EVT_TRACKBALL_UP);
  gpio_isr_handler_add(BOARD_TRACKBALL_DOWN_PIN, td_trackball_int,
                       (void *)EVT_TRACKBALL_DOWN);
  gpio_isr_handler_add(BOARD_TRACKBALL_LEFT_PIN, td_trackball_int,
                       (void *)EVT_TRACKBALL_LEFT);
  gpio_isr_handler_add(BOARD_TRACKBALL_RIGHT_PIN, td_trackball_int,
                       (void *)EVT_TRACKBALL_RIGHT);

  return ESP_OK;
}
