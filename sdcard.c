#include <board.h>
#include <sdcard.h>

#define TAG "SDCARD"

// inspired by
// https://github.com/ByteWelder/Tactility/tree/main/boards/lilygo_tdeck

/**
 * Before we can initialize the sdcard's SPI communications, we have to set all
 * other SPI pins on the board high.
 * See https://github.com/espressif/esp-idf/issues/1597
 * See
 * https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino
 * @return success result
 */
esp_err_t td_sdcard_init(void *ctx, const char *mount_point) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_sdcard_t *SDCard = Board->SDCard;

  ESP_LOGI(TAG, "Initializing...");

  if (SDCard != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  SDCard = malloc(sizeof(td_sdcard_t));
  if (SDCard == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->SDCard = SDCard;

  spi_device_interface_config_t sdspi_config = {
      .clock_speed_hz = BOARD_SDCARD_CLOCK_SPEED,
      .mode = 0,
      .spics_io_num = BOARD_SDCARD_CS_PIN,
      .queue_size = 10,
  };

  spi_device_handle_t handle;
  esp_err_t ret = spi_bus_add_device(BOARD_SPI, &sdspi_config, &handle);

  bzero((char*)SDCard->mount_point, STR_SIZE_MEDIUM);
  strncpy((char*)SDCard->mount_point, mount_point, STR_SIZE_MEDIUM - 1);

  if (ret != ESP_OK) {
    return ret;
  }

  SDCard->dev = handle;
  SDCard->initialized = true;
  return ESP_OK;
}

esp_err_t sdcard_mount(void *ctx, const char *mount_point) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_sdcard_t *SDCard = Board->SDCard;

  if (SDCard == NULL) {
    ESP_LOGE(TAG, "Could not mount");
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = ESP_OK;

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = BOARD_SPI;

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = BOARD_SDCARD_CS_PIN;
  slot_config.host_id = host.slot;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = true,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};

  sdmmc_card_t *card;
  err = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config,
                                &card);

  if (err == ESP_OK) {
    ESP_LOGI(TAG, "SD card mounted.");
    SDCard->card = card;
    SDCard->mounted = true;
  } else {
    ESP_LOGE(TAG, "Failed to initialize the SD card: %s", esp_err_to_name(err));
  }
  return err;
}

esp_err_t sdcard_unmount(void *ctx) {
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_sdcard_t *SDCard = Board->SDCard;

  if (SDCard == NULL || !SDCard->mounted) {
    ESP_LOGW(TAG, "Could not unmount");
    return ESP_FAIL;
  }

  esp_err_t err =
      esp_vfs_fat_sdcard_unmount(Board->SDCard->mount_point, Board->SDCard->card);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Unmount failed for %s", Board->SDCard->mount_point);
    return err;
  }
  SDCard->mounted = false;
  return err;
}

esp_err_t sdcard_is_mounted(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;
  // TODO: use a semaphore lock since SD, Radio and LCD share the same SPI bus
  // if ( xSemaphoreTake( sd_radio_display_lock, 100 ) ) {
  return sdmmc_get_status(Board->SDCard->card);
  // xSemaphoreGive( sd_radio_display_lock );
  //  return result;
  //} else {
  //    return false;
  //}
}
