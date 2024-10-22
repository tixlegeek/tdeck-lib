
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <sys/stat.h>

#include <cJSON.h>
#include <utils.h>

#include <jsonconfig.h>
const static char *TAG = "CONFIG";

// json -> cJSON
static esp_err_t json_decode(const char *jsonStr, cJSON **jsonObj) {
    *jsonObj = cJSON_ParseWithOpts(jsonStr, NULL, 1);
    if (*jsonObj == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON string: %s", jsonStr);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t config_get_credential(cJSON *jsonObj, char *ssidStr, size_t ssidLen,
                                       char *passwordStr, size_t passLen) {
    if (!cJSON_IsObject(jsonObj)) {
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(jsonObj, "ssid");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(jsonObj, "password");

    if (cJSON_IsString(ssid) && cJSON_IsString(password)) {
        strncpy(ssidStr, ssid->valuestring, ssidLen);
        strncpy(passwordStr, password->valuestring, passLen);
        return ESP_OK;
    }

    return ESP_ERR_INVALID_ARG;
}

static esp_err_t json_decode_struct(cJSON *jsonObj, td_config_t *config) {
    if (jsonObj == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Decoded JSON: %s", cJSON_Print(jsonObj));

    cJSON *net = cJSON_GetObjectItemCaseSensitive(jsonObj, "net");
    if (cJSON_IsObject(net)) {
        cJSON *ap = cJSON_GetObjectItemCaseSensitive(net, "ap");
        if (cJSON_IsObject(ap)) {
            if (config_get_credential(ap, config->net.ap.ssid, sizeof(config->net.ap.ssid),
                                      config->net.ap.password, sizeof(config->net.ap.password)) == ESP_OK) {
                ESP_LOGI(TAG, "AP config: SSID: %s, Password: %s", config->net.ap.ssid, config->net.ap.password);
            } else {
                ESP_LOGE(TAG, "Invalid or missing AP config");
            }
        } else {
            ESP_LOGE(TAG, "AP config not present");
        }

        cJSON *sta = cJSON_GetObjectItemCaseSensitive(net, "sta");
        if (cJSON_IsObject(sta)) {
            if (config_get_credential(sta, config->net.sta.ssid, sizeof(config->net.sta.ssid),
                                      config->net.sta.password, sizeof(config->net.sta.password)) == ESP_OK) {
                ESP_LOGI(TAG, "STA config: SSID: %s, Password: %s", config->net.sta.ssid, config->net.sta.password);
            } else {
                ESP_LOGE(TAG, "Invalid or missing STA config");
            }
        } else {
            ESP_LOGE(TAG, "STA config not present");
        }
    } else {
        ESP_LOGE(TAG, "Network configuration is missing");
    }

    return ESP_OK;
}

static esp_err_t json_encode(cJSON *jsonObj, char **jsonStr) {
    if (jsonObj == NULL || jsonStr == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *jsonStr = cJSON_Print(jsonObj);
    if (*jsonStr == NULL) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Encoded JSON: %s", *jsonStr);
    return ESP_OK;
}

static esp_err_t json_encode_struct(const td_config_t *config, cJSON *jsonObj) {
    if (config == NULL || jsonObj == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Example JSON creation for monitor field, extend as needed
    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL) {
        return ESP_ERR_NO_MEM;
    }

    cJSON_AddItemToObject(jsonObj, "monitor", monitor);
    return ESP_OK;
}

esp_err_t td_load(const char *filename, td_config_t *config) {
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

    if (fread(jsonStr, sb.st_size, 1, fd) != 1) {
        ESP_LOGE(TAG, "Failed to read JSON data");
        fclose(fd);
        free(jsonStr);
        return ESP_FAIL;
    }
    jsonStr[sb.st_size] = '\0';

    fclose(fd);

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

esp_err_t td_save(const char *filename, td_config_t *config) {
  /*json_encode_struct( td_config_t *config, cJSON* obj);
  json_encode( cJSON* obj, char *json);*/
  return ESP_OK;

  // write json
}
