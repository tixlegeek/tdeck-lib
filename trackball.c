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

static void IRAM_ATTR td_trackball_int(void *_evt) {
    uint8_t evt = (uint8_t)_evt;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xEventGroupSetBitsFromISR(td_trackball_evtgroup, evt, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(); // Yield if a higher priority task needs to run
    }
}

char *td_trackball_evt2str(uint8_t evt) {
  UNUSED(evt);
  return td_trackball_evt_str[0];
}

void td_trackball_task(void *ctx) {
  assert(ctx != NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_trackball_t *Trackball = Board->Trackball;
    UBaseType_t uxHighWaterMark;
  if (Trackball == NULL) {
    vTaskDelete(NULL);
  }
  EventBits_t evtbits = NULL;
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10));
    evtbits = xEventGroupWaitBits(
        td_trackball_evtgroup,
        EVT_TRACKBALL_UP | EVT_TRACKBALL_DOWN | EVT_TRACKBALL_LEFT |
            EVT_TRACKBALL_RIGHT, // The bits to wait for
        pdFALSE,                  // Clear bits on exit
        pdFALSE,                 // Wait for both bits
        portMAX_DELAY            // Wait indefinitely
    );
    // Attente des données dans la queue
    if (evtbits & EVT_TRACKBALL_UP) {
      printf("EVT_UP %d\n", uxTaskGetStackHighWaterMark( NULL ));
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_UP);
    }
    if (evtbits & EVT_TRACKBALL_DOWN) {
      printf("EVT_DOWN %d\n", uxTaskGetStackHighWaterMark( NULL ));
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_DOWN);
    }
    if (evtbits & EVT_TRACKBALL_LEFT) {
      printf("EVT_LEFT %d\n", uxTaskGetStackHighWaterMark( NULL ));
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_LEFT);
    }
    if (evtbits & EVT_TRACKBALL_RIGHT) {
      printf("EVT_RIGHT %d\n", uxTaskGetStackHighWaterMark( NULL ));
      xEventGroupClearBits(td_trackball_evtgroup, EVT_TRACKBALL_RIGHT);
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
