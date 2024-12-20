#include "audio_player.h"
#include "esp_check.h"
#include "esp_log.h"
#include <board.h>
#include <speaker.h>
const static char *TAG = "AUDIO";

static i2s_chan_handle_t i2s_tx_chan = NULL;
static td_board_t *board_ = NULL;

static esp_err_t bsp_i2s_write(void *audio_buffer, size_t len,
                               size_t *bytes_written, uint32_t timeout_ms) {
  esp_err_t ret = ESP_OK;
  ret = i2s_channel_write(i2s_tx_chan, (char *)audio_buffer, len, bytes_written,
                          timeout_ms);
  return ret;
}

static esp_err_t bsp_i2s_reconfig_clk(uint32_t rate, uint32_t bits_cfg,
                                      i2s_slot_mode_t ch) {
                                        ESP_LOGI(TAG, "bsp_i2s_reconfig_clk %ld", rate);
  esp_err_t ret =  td_speaker_reconfig_clk(board_, rate, bits_cfg, ch);
  return ret;
}

static esp_err_t audio_mute_function(AUDIO_PLAYER_MUTE_SETTING setting) {
  ESP_LOGI(TAG, "mute setting %d", setting);
  return ESP_OK;
}

// Mon comptable rêve de bible et de gros chiffres

static audio_player_callback_event_t expected_event;
static QueueHandle_t event_queue;

static void audio_player_callback(audio_player_cb_ctx_t *ctx)
{
  switch (ctx->audio_event) {
   case AUDIO_PLAYER_CALLBACK_EVENT_IDLE: /**< Player is idle, not playing audio */
       ESP_LOGI(TAG, "IDLE");
       break;
   case AUDIO_PLAYER_CALLBACK_EVENT_PLAYING:
   case AUDIO_PLAYER_CALLBACK_EVENT_COMPLETED_PLAYING_NEXT:
       ESP_LOGI(TAG, "%s", (AUDIO_PLAYER_CALLBACK_EVENT_PLAYING == ctx->audio_event) ? "Play" : "Next");
       break;
   case AUDIO_PLAYER_CALLBACK_EVENT_PAUSE:
       ESP_LOGI(TAG, "PAUSE");
       break;
   case AUDIO_PLAYER_CALLBACK_EVENT_SHUTDOWN:
       ESP_LOGI(TAG, "SHUTDOWN");
       break;
   case AUDIO_PLAYER_CALLBACK_EVENT_UNKNOWN_FILE_TYPE:
       ESP_LOGI(TAG, "UNKNOWN FILE");
       break;
   case AUDIO_PLAYER_CALLBACK_EVENT_UNKNOWN:
       ESP_LOGI(TAG, "UNKNOWN");
       break;
   }
}

int td_audio_getState(void* ctx){
  audio_player_state_t state = audio_player_get_state();
  return (int)state;
}

esp_err_t td_audio_play(void* ctx, FILE* fp){
  if (!fp) {
      ESP_LOGE(TAG, "unable to open 'file'");
      return ESP_OK;
  } else {
      ESP_LOGI(TAG, "open 'file'");
  }

  return audio_player_play(fp);
}

int td_audio_still_playing(void *ctx){
  UNUSED(ctx);
  audio_player_callback_event_t event;
  if(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)) == pdPASS)
    return 0;
  else
    return 1;
}


esp_err_t td_audio_play_(void *ctx, char *path){
    UNUSED(path);
    assert(ctx);
    td_board_t *Board = (td_board_t *)ctx;
    esp_err_t ret = ESP_OK;
    audio_player_state_t state = audio_player_get_state();
    assert(state == AUDIO_PLAYER_STATE_IDLE);
    audio_player_callback_event_t event;

    // -1 due to the size being 1 byte too large, I think because end is the byte
    // immediately after the last byte in the memory but I'm not sure - cmm
    // 2022-08-20
    //
    // Suppression as these are linker symbols and cppcheck doesn't know how to
    // ensure they are the same object cppcheck-suppress comparePointers

    FILE *fp = fopen("/sdcard/HONTEU~1.MP3", "rb");
    assert(fp);
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PLAYING;
    ret = audio_player_play(fp);
    ESP_ERROR_CHECK(ret);

    // wait for playing event to arrive
    assert(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))==pdPASS);

    // confirm state is playing
    state = audio_player_get_state();
    assert(state== AUDIO_PLAYER_STATE_PLAYING);

    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PAUSE;
    ret = audio_player_pause();
    ESP_ERROR_CHECK(ret);

    // wait for paused event to arrive
    assert(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))==pdPASS);

    state = audio_player_get_state();
    assert(state == AUDIO_PLAYER_STATE_PAUSE);

    ////////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_PLAYING;
    ret = audio_player_resume();
    ESP_ERROR_CHECK(ret);

    // wait for paused event to arrive
    assert(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)) == pdPASS);

    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_IDLE;

    // the track is 16 seconds long so lets wait a bit here
    int sleep_seconds = 16;
    ESP_LOGI(TAG, "sleeping for %d seconds for playback to complete",
             sleep_seconds);
    vTaskDelay(pdMS_TO_TICKS(sleep_seconds * 1000));

    // wait for idle event to arrive
    assert(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)) == pdPASS);

    state = audio_player_get_state();
    assert(state==AUDIO_PLAYER_STATE_IDLE);

    ///////////////
    expected_event = AUDIO_PLAYER_CALLBACK_EVENT_SHUTDOWN;
    ret = audio_player_delete();
    ESP_ERROR_CHECK(ret);

    // wait for idle event to arrive
    assert(xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100)) == pdPASS);

    state = audio_player_get_state();
    assert(state == AUDIO_PLAYER_STATE_SHUTDOWN);

    vQueueDelete(event_queue);
    return ESP_OK;
}

esp_err_t td_audio_init(void *ctx) {
  assert(ctx);
  td_board_t *Board = (td_board_t *)ctx;
  i2s_tx_chan = Board->Speaker->dev;
  board_ = Board;
  esp_err_t ret = ESP_OK;

  audio_player_callback_event_t event;
  // Assign static i2s channel to the board's speaker

  audio_player_config_t config = {.mute_fn = audio_mute_function,
                                  .write_fn = bsp_i2s_write,
                                  .clk_set_fn = bsp_i2s_reconfig_clk,
                                  .priority = 5,
                                  .coreID = 1};

  ret = audio_player_new(config);
  ESP_ERROR_CHECK(ret);

  event_queue = xQueueCreate(1, sizeof(audio_player_callback_event_t));
  if(event_queue == NULL){
    ESP_LOGE(TAG, "Could not allocate playback queue.");
    return ESP_FAIL;
  }

  ret = audio_player_callback_register(audio_player_callback, NULL);
  if(ret != ESP_OK){
    ESP_LOGE(TAG, "Could not register audio player callback.");
    return ret;
  }

  return ESP_OK;
}
