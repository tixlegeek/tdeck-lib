#include <board.h>
#include <trackball.h>

const static char *TAG = "TRACKBALL";
static QueueHandle_t td_trackball_queue = NULL;

static char *td_trackball_evt_str[] = {
  "EVT_UP",
  "EVT_DOWN",
  "EVT_LEFT",
  "EVT_RIGHT",
};

static void IRAM_ATTR td_trackball_int(void *_evt) {
  td_trackball_evt_t evt = (td_trackball_evt_t)_evt;
  xQueueSendFromISR(td_trackball_queue, &evt, NULL);
}

char *td_trackball_evt2str(td_trackball_evt_t evt){
  return td_trackball_evt_str[evt];
}

void td_trackball_event(void *pvParameters)
{
    td_trackball_evt_t evt;

    while (1)
    {
        // Attente des données dans la queue
        if (xQueueReceive(td_trackball_queue, &evt, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "Reçu : %s", td_trackball_evt2str(evt));
        }
        else
        {
            ESP_LOGE(TAG, "Erreur de réception de la queue");
        }
    }
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
      .pull_down_en = 0, // disable pull-down mode
      .pull_up_en = 0,   // disable pull-up mode
  };

  gpio_config(&io_conf);

  td_trackball_queue = xQueueCreate(10, sizeof(td_trackball_evt_t));

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
