#include <board.h>
#include <speaker.h>

const static char *TAG = "SPEAKER";


esp_err_t td_speaker_init(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;

  i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &Board->Speaker.dev, NULL));

  // default PCM S16LE 44100
  static i2s_std_config_t tx_std_cfg = { // << config is static
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg =
      {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = BOARD_SPEAKER_BCLK_PIN,
        .ws =   BOARD_SPEAKER_WS_PIN,
        .dout = BOARD_SPEAKER_DIN_PIN,
        .din =  I2S_GPIO_UNUSED,
        .invert_flags =
        {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
      },
  };

  Board->Speaker.tx_cfg = &tx_std_cfg; // << stored for later reconfig

  return i2s_channel_init_std_mode(Board->Speaker.dev, &tx_std_cfg);
}

