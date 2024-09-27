#include <board.h>
#include <speaker.h>

const static char *TAG = "SPEAKER";

esp_err_t td_speaker_init(void *ctx) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_speaker_t *Speaker = Board->Speaker;
  ESP_LOGI(TAG, "Initializing...");

  if (Board->Speaker != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  Speaker = malloc(sizeof(td_speaker_t));
  if (Speaker == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->Speaker = Speaker;

  i2s_chan_config_t tx_chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &Speaker->dev, NULL));
  return td_speaker_configure(Board, BOARD_SPEAKER_DEFAULT_SAMPLERATE);
}

esp_err_t td_speaker_configure(void *ctx, uint32_t sample_rate) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_speaker_t *Speaker = Board->Speaker;

  if (Speaker==NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  static i2s_std_config_t tx_std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(BOARD_SPEAKER_DEFAULT_SAMPLERATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                      I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = BOARD_SPEAKER_BCLK_PIN,
              .ws = BOARD_SPEAKER_WS_PIN,
              .dout = BOARD_SPEAKER_DIN_PIN,
              .din = I2S_GPIO_UNUSED,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };
  tx_std_cfg.clk_cfg.sample_rate_hz = sample_rate;

  return i2s_channel_init_std_mode(Speaker->dev, &tx_std_cfg);
}

void td_speaker_task(void *ctx) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  UNUSED(Board);
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}
