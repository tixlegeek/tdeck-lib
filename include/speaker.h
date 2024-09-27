#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include <freertos/FreeRTOS.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

#define SAMPLE_RATE_44K    44100 // DVD-A, DTS-CD
#define SAMPLE_RATE_48K     48000 // CD, ATSC
#define SAMPLE_RATE_88K     88200 // DVD-V, SACD
#define SAMPLE_RATE_96K     96000 // DVD-A, Super Audio CD (SACD)
#define SAMPLE_RATE_176K    176400 // DTS-HD
#define SAMPLE_RATE_192K    192000 // Blu-ray, HD DVD, ATSC, HDCD, DTS-CD,SACD
#define SAMPLE_RATE_352K    352800 // Super CD (Super CD+G)
#define SAMPLE_RATE_384K    384000 // HDCD, DTS-HD
#define SAMPLE_RATE_16K     16000 // Low band audio
#define SAMPLE_RATE_22K     22000 // AM radio broadcast quality


typedef struct td_speaker_t {
  bool initialized;
  i2s_chan_handle_t dev;
  i2s_std_config_t *tx_cfg;
} td_speaker_t;

esp_err_t td_speaker_init(void *ctx);
esp_err_t td_speaker_configure(void *ctx, uint32_t sample_rate);
esp_err_t td_speaker_reconfig_clk(void *ctx, uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);

#endif /* end of include guard: _SPEAKER_H_ */
