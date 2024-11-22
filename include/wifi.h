#ifndef _WIFI_H_
#define _WIFI_H_

#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <esp_err.h>

#define SSID_LENGTH 32
#define PASSWORD_LENGTH 64
#define CONNECTED_BIT (1<<0)

typedef struct {
    esp_netif_t *ap_netif;
    esp_netif_t *sta_netif;
    EventGroupHandle_t event_group;
} td_wifi_t;
#include <board.h>

esp_err_t td_wifi_init(void* ctx);

#endif /* end of include guard: _WIFI_H_ */
