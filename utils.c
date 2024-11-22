#include <utils.h>
#include <dirent.h>
#include <stdio.h>
#include <board.h>

esp_err_t td_ls( const char *directory ) {
  struct dirent *entity;
  // BOARD_SDCARD_MOUNT_POINT
  DIR *dir = opendir(directory);
  if (dir) {
    while ((entity = readdir(dir)) != NULL) {
      switch (entity->d_type) {
      case DT_DIR: // If entity points to a directory
        ESP_LOGW(__FUNCTION__, "+ %s", entity->d_name);
        break;
      case DT_REG: // If entity points to a file?
        ESP_LOGW(__FUNCTION__, "- %s", entity->d_name);
        break;
      default:
        ESP_LOGW(__FUNCTION__, "? %s", entity->d_name);
        break;
      }
    }
    closedir(dir);
  } else {
    ESP_LOGE(__FUNCTION__, "Could not open dir \"%s\"", BOARD_SDCARD_MOUNT_POINT);
    return ESP_FAIL;
  }
  return ESP_OK;
}
