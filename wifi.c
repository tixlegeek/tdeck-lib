#include <wifi.h>

const static char *TAG = "WIFI";

void wifi_apsta(void* ctx, int timeout_ms) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_wifi_t *Wifi = Board->Wifi;
  td_config_t *Config = Board->Config;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &Config->net.ap));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &Config->net.sta));
  ESP_ERROR_CHECK(esp_wifi_start());


  ESP_ERROR_CHECK(esp_wifi_connect());
  int bits = xEventGroupWaitBits(Wifi->event_group, CONNECTED_BIT, pdFALSE,
                                 pdTRUE, timeout_ms / portTICK_PERIOD_MS);

}

void td_wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  switch (event_id) {
  case WIFI_EVENT_AP_STACONNECTED:
    ESP_LOGI(TAG, "WIFI AP STA CONNECTED, STARTING SERVER.");
    break;
  case WIFI_EVENT_AP_STADISCONNECTED:
    // connect_handler(&https_server);
    break;
  case WIFI_EVENT_STA_CONNECTED:
    break;
  case WIFI_EVENT_STA_DISCONNECTED:
    break;
  default:
    break;
  }
}

void td_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                      void *event_data) {
  switch (event_id) {
  case IP_EVENT_STA_GOT_IP:
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    break;
  case IP_EVENT_STA_LOST_IP:
    break;
  default:
    break;
  }
}

esp_err_t td_wifi_init(void* ctx) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_wifi_t *Wifi = Board->Wifi;
  td_config_t *Config = Board->Config;

  ESP_LOGI(TAG, "Initializing...");

  if (Board->Wifi != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  Wifi = malloc(sizeof(td_wifi_t));
  if (Config == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->Wifi = Wifi;

  esp_log_level_set("wifi", ESP_LOG_ERROR);

  ESP_ERROR_CHECK(esp_netif_init());
  Wifi->event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  Wifi->sta_netif = esp_netif_create_default_wifi_sta();
  assert(Wifi->sta_netif);
  // Makes fingerprinting a bit more fun
  ESP_LOGI(TAG, "[*] Changing STA hostname to %s", Config->net.sta_hostname);
  ESP_ERROR_CHECK(esp_netif_set_hostname(Wifi->sta_netif, Config->net.sta_hostname));

  Wifi->ap_netif = esp_netif_create_default_wifi_ap();
  assert(Wifi->ap_netif);

  // Makes fingerprinting a bit more fun
  ESP_LOGI(TAG, "[*] Changing AP hostname to %s", Config->net.ap_hostname);
  ESP_ERROR_CHECK(esp_netif_set_hostname(Wifi->ap_netif, Config->net.ap_hostname));

  //configure_mac_wifi(ap_netif, sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // AP mode client connected
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &td_wifi_event_handler, NULL));
  // AP mode client disconnected
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &td_wifi_event_handler, NULL));
  // Station mode connect
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &td_wifi_event_handler, NULL));
  // Station mode disconnect
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &td_wifi_event_handler, NULL));

  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &td_ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP,
                                             &td_ip_event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));

  ESP_ERROR_CHECK(esp_wifi_start());

//  esp_netif_ip_info_t ap_ip;
//  esp_netif_get_ip_info(Wifi->ap_netif, &ap_ip);

  return ESP_OK;
}
