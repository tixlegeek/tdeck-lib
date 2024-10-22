#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_
#include <stdio.h>


typedef struct {
  struct  {
    struct  {
      char ssid[32];
      char password[64];
      char lanIp[16];
    } ap;
    struct  {
      char ssid[32];
      char password[64];
    } sta;
  } net;
  struct  {
    uint16_t baudrate;
  } gps;

} td_config_t;

esp_err_t td_load(const char *filename, td_config_t *config);
esp_err_t td_save(const char *filename, td_config_t *config);

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
