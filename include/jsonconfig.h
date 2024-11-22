#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_
#include <stdio.h>
#include <wifi.h>

typedef struct {
  struct  {
    wifi_ap_config_t ap;
    char ap_hostname[64];
    wifi_sta_config_t sta;
    char sta_hostname[64];
  } net;
  struct  {
    int baudrate;
  } gps;
} td_config_t;

esp_err_t td_config_load(const char *filename, td_config_t *config);
esp_err_t td_config_save(const char *filename, td_config_t *config);
esp_err_t td_config_default(td_config_t *config);
esp_err_t td_config_init(void *ctx);

#endif /* end of include guard: _CONFIG_FILE_H_ */

/*
{
  "net":{
    "AP":{
      "ssid" ssid,
      "password": pwd
      "lanIp": iplan
    }
  }
  "gps":{
    "baudrate":38400
  }
}
*/
