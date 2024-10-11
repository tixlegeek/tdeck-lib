#ifndef _AUDIO_H_
#define _AUDIO_H_
esp_err_t td_audio_getState(void* ctx);
esp_err_t td_audio_play(void* ctx, FILE* fp);
int td_audio_still_playing(void *ctx);
esp_err_t td_audio_init(void *ctx);


#endif /* end of include guard: _AUDIO_H_ */
