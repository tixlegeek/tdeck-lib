
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <sys/stat.h>

#include <board.h>
#include <cJSON.h>
#include <jsonconfig.h>
#include <utils.h>
#include <esp_wifi.h>
#include <esp_netif.h>

const static char *TAG = "CONFIG";

/*

*/
static wifi_auth_mode_t authStr2Enum(const char *authStr) {
  if( strcmp( "WPA2_PSK", authStr )){
    return WIFI_AUTH_WPA2_PSK;
  }
  else if( strcmp( "WPA_WPA2_PSK", authStr )){
    return WIFI_AUTH_WPA_WPA2_PSK;
  }
  else if( strcmp( "WPA3_PSK", authStr )){
    return WIFI_AUTH_WPA3_PSK;
  }
  else if( strcmp( "WPA2_WPA3_PSK", authStr )){
    return WIFI_AUTH_WPA2_WPA3_PSK;
  }
  else if( strcmp( "WPA3_ENT_192", authStr )){
    return WIFI_AUTH_WPA3_ENT_192;
  }
  else if( strcmp( "ENTERPRISE", authStr )){
    return WIFI_AUTH_ENTERPRISE;
  }
  else if( strcmp( "WPA_PSK", authStr )){
    return WIFI_AUTH_WPA_PSK;
  }
  if( strcmp( "OPEN", authStr )){
    return WIFI_AUTH_OPEN;
  }
  else if( strcmp( "OWE", authStr )){
    return WIFI_AUTH_OWE;
  }
  else if( strcmp( "WEP", authStr )){
    return WIFI_AUTH_WEP;
  }
  return WIFI_AUTH_WPA2_PSK;
}

/**
  brief: Converts a json string into a cJSON Object

*/
static esp_err_t json_decode(const char *jsonStr, cJSON **jsonObj) {
  *jsonObj = cJSON_ParseWithOpts(jsonStr, NULL, 1);
  if (*jsonObj == NULL) {
    ESP_LOGE(TAG, "Failed to parse JSON string: %s", jsonStr);
    return ESP_FAIL;
  }
  return ESP_OK;
}

/**
  brief: Extract Wifi essid and password from a cJSON object and fills
  ssidStr and passwordStr with the content.
*/
static esp_err_t config_read_credential(cJSON *jsonObj, uint8_t *ssidStr,
                                        uint8_t *passwordStr) {
  if (!cJSON_IsObject(jsonObj)) {
    return ESP_ERR_INVALID_ARG;
  }

  cJSON *ssid = cJSON_GetObjectItemCaseSensitive(jsonObj, "ssid");
  cJSON *password = cJSON_GetObjectItemCaseSensitive(jsonObj, "password");

  if (cJSON_IsString(ssid) && cJSON_IsString(password)) {
    strncpy((char*)ssidStr, ssid->valuestring, SSID_LENGTH - 1);
    strncpy((char*)passwordStr, password->valuestring, PASSWORD_LENGTH - 1);
    return ESP_OK;
  }

  return ESP_ERR_INVALID_ARG;
}

/**
  brief: Extract Wifi configuration from a cJSON object and fills
  a td_config_net_ap_t struct passed as pointer.
*/
static esp_err_t config_read_wifiApConfig(cJSON *jsonObj,
                                          wifi_ap_config_t *config_ap) {
  if (!cJSON_IsObject(jsonObj)) {
    return ESP_ERR_INVALID_ARG;
  }

  cJSON *auth = cJSON_GetObjectItemCaseSensitive(jsonObj, "auth");
  cJSON *channel = cJSON_GetObjectItemCaseSensitive(jsonObj, "channel");
  cJSON *maxsta = cJSON_GetObjectItemCaseSensitive(jsonObj, "maxsta");

  if (cJSON_IsString(auth)) {
    config_ap->authmode = authStr2Enum(auth->valuestring);
  }
  if (cJSON_IsNumber(channel)) {
    config_ap->channel = (uint8_t)channel->valueint;
  }
  if (cJSON_IsNumber(maxsta)) {
    config_ap->max_connection = (uint8_t)maxsta->valueint;
  }

  return ESP_OK;
}

/**
  brief: Converts a cJSON Structure into a configuration structure by grabbing
  every information needed.

*/
static esp_err_t json_decode_struct(cJSON *jsonObj, td_config_t *config) {
  if (jsonObj == NULL || config == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  cJSON *net = cJSON_GetObjectItemCaseSensitive(jsonObj, "net");
  if (cJSON_IsObject(net)) {
    cJSON *ap = cJSON_GetObjectItemCaseSensitive(net, "ap");
    if (cJSON_IsObject(ap)) {
      if (config_read_credential(ap, config->net.ap.ssid, config->net.ap.password) ==
          ESP_OK) {
        ESP_LOGI(TAG, "AP config: SSID: %s, Password: %s", config->net.ap.ssid,
                 config->net.ap.password);
      } else {
        ESP_LOGE(TAG, "Invalid or missing AP config");
      }
    } else {
      ESP_LOGE(TAG, "AP config not present");
    }

    cJSON *sta = cJSON_GetObjectItemCaseSensitive(net, "sta");
    if (cJSON_IsObject(sta)) {
      if (config_read_credential(sta, config->net.sta.ssid,
                                 config->net.sta.password) == ESP_OK) {
        ESP_LOGI(TAG, "STA config: SSID: %s, Password: %s",
                 config->net.sta.ssid, config->net.sta.password);
      } else {
        ESP_LOGE(TAG, "Invalid or missing STA config");
      }

      if (config_read_wifiApConfig(ap, &config->net.sta) != ESP_OK) {
        ESP_LOGE(TAG, "STA config read failed");
      }

    } else {
      ESP_LOGE(TAG, "STA config not present");
    }
  } else {
    ESP_LOGE(TAG, "Network configuration is missing");
  }

  return ESP_OK;
}

/**
  brief: Converts a cJSON Object into a json string.
*/
static esp_err_t json_encode(cJSON *jsonObj, char **jsonStr) {
  if (jsonObj == NULL || jsonStr == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  *jsonStr = cJSON_Print(jsonObj);
  if (*jsonStr == NULL) {
    return ESP_ERR_NO_MEM;
  }

  return ESP_OK;
}

/**
  brief: write wifi essid and password into a cJSON Object.
*/
static esp_err_t config_write_credential(cJSON *jsonObj, char *ssidStr,
                                         char *passwordStr) {
  if (!cJSON_IsObject(jsonObj)) {
    ESP_LOGE(TAG, "cJSON Object not allocated");
    return ESP_ERR_INVALID_ARG;
  }

  if (ssidStr == NULL || passwordStr == NULL) {
    ESP_LOGE(TAG, "Empty pssword/ssid strings");
    return ESP_ERR_INVALID_ARG;
  }

  cJSON *ssid = cJSON_CreateString(ssidStr);
  if (ssid != NULL) {
    cJSON_AddItemToObject(jsonObj, "ssid", ssid);
  } else {
    ESP_LOGE(TAG, "Could not create string for ssid");
  }

  cJSON *password = cJSON_CreateString(passwordStr);
  if (password != NULL) {
    cJSON_AddItemToObject(jsonObj, "password", password);
  } else {
    ESP_LOGE(TAG, "Could not create string for password");
  }

  return ESP_OK;
}

/**
  brief: Converts a configuration structure into a cJSON Object
*/
static esp_err_t json_encode_struct(const td_config_t *config, cJSON *jsonObj) {

  if (config == NULL || jsonObj == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Example JSON creation for monitor field, extend as needed
  cJSON *net = cJSON_CreateObject();
  cJSON *ap = cJSON_CreateObject();
  if (config_write_credential(ap, config->net.ap.ssid,
                              config->net.ap.password) == ESP_OK) {
    cJSON_AddItemToObject(net, "ap", ap);
  }

  cJSON *sta = cJSON_CreateObject();
  if (config_write_credential(sta, config->net.sta.ssid,
                              config->net.sta.password) == ESP_OK) {
    cJSON_AddItemToObject(net, "sta", sta);
  }
  cJSON_AddItemToObject(jsonObj, "net", net);

  cJSON *gps = cJSON_CreateObject();
  cJSON *baudrate = cJSON_CreateNumber(config->gps.baudrate);
  cJSON_AddItemToObject(gps, "baudrate", baudrate);
  cJSON_AddItemToObject(jsonObj, "gps", gps);

  return ESP_OK;
}

/**
  brief: High level function to extract a configuration structure from a json
  file.
*/
esp_err_t td_config_load(const char *filename, td_config_t *config) {

  if (filename == NULL || config == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  struct stat sb;
  if (stat(filename, &sb) == -1) {
    ESP_LOGE(TAG, "Failed to get file info: %s", filename);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Configuration file size: %ld", sb.st_size);

  char *jsonStr = malloc(sb.st_size + 1);
  if (jsonStr == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for JSON");
    return ESP_ERR_NO_MEM;
  }

  FILE *fd = fopen(filename, "r");
  if (fd == NULL) {
    ESP_LOGE(TAG, "Failed to open configuration file: %s", filename);
    free(jsonStr);
    return ESP_ERR_NOT_FOUND;
  }

  if (fread(jsonStr, 1, sb.st_size, fd) != sb.st_size) {
    ESP_LOGE(TAG, "Failed to read JSON data");
    fclose(fd);
    free(jsonStr);
    return ESP_FAIL;
  }
  jsonStr[sb.st_size] = '\0';
  fclose(fd);
  ESP_LOGW(__FUNCTION__,"%s", jsonStr);
  cJSON *jsonObj = NULL;
  esp_err_t err = json_decode(jsonStr, &jsonObj);
  free(jsonStr);

  if (err != ESP_OK) {
    return err;
  }

  err = json_decode_struct(jsonObj, config);
  cJSON_Delete(jsonObj);

  return err;
}
/**
  brief: High level function write a json file from a configuration structure.
*/
esp_err_t td_config_save(const char *filename, td_config_t *config) {
  cJSON *jsonObj = cJSON_CreateObject();
  char *jsonStr = NULL;

  if (json_encode_struct(config, jsonObj) != ESP_OK) {
    ESP_LOGE(TAG, "Could encode struct into cJSON");
    cJSON_Delete(jsonObj);
    return ESP_FAIL;
  }

  if (json_encode(jsonObj, &jsonStr) != ESP_OK) {
    ESP_LOGE(TAG, "Could not encode cJSON to text");
    cJSON_Delete(jsonObj);
    return ESP_FAIL;
  }

  size_t jsonStrLen = strlen(jsonStr);
  FILE *fd = fopen(filename, "w+");

  if (fd == NULL) {
    ESP_LOGE(TAG, "Could not open file named \"%s\"", filename);
    cJSON_Delete(jsonObj);
    free(jsonStr);
    return ESP_ERR_NOT_ALLOWED;
  }

  if (fwrite(jsonStr, 1, jsonStrLen, fd) != jsonStrLen) {
    ESP_LOGE(TAG, "Could not write to file named \"%s\"", filename);
    cJSON_Delete(jsonObj);
    free(jsonStr);
    return ESP_ERR_NO_MEM;
  }

  ESP_LOGI(TAG, "Configuration has been written!");

  fclose(fd);
  return ESP_OK;
}

esp_err_t td_config_default(td_config_t *config) {
  if (config == NULL) {
    ESP_LOGE(TAG, "Configuration struct not allocated.");
    return ESP_FAIL;
  }
  snprintf((char*)config->net.ap.ssid, 32, "DefaultSsid");
  snprintf((char*)config->net.ap.password, 32, "DefaultPassword");
  snprintf((char*)config->net.sta.ssid, 32, "DefaultSsid");
  snprintf((char*)config->net.sta.password, 32, "DefaultPassword");
  config->gps.baudrate = 115200;
  return ESP_OK;
}

esp_err_t td_config_init(void *ctx) {
  assert(ctx != NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_config_t *Config = Board->Config;
  ESP_LOGI(TAG, "Initializing...");

  if (Board->Config != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  Config = malloc(sizeof(td_config_t));
  if (Config == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->Config = Config;

  return ESP_OK; // td_speaker_configure(Board,
                 // BOARD_SPEAKER_DEFAULT_SAMPLERATE);
}
// CPT
