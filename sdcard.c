#include <board.h>
#include <sdcard.h>

#define TAG "tdeck_sdcard"

esp_err_t td_sdcard_init(void *ctx) {
    return sdcard_init() ? ESP_OK : ESP_ERR_INVALID_ARG;
}


// inspired by https://github.com/ByteWelder/Tactility/tree/main/boards/lilygo_tdeck

/**
 * Before we can initialize the sdcard's SPI communications, we have to set all
 * other SPI pins on the board high.
 * See https://github.com/espressif/esp-idf/issues/1597
 * See https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino
 * @return success result
 */
bool sdcard_init() {
    ESP_LOGD(TAG, "init");

    gpio_config_t config = {
        .pin_bit_mask = BIT64(BOARD_SDCARD_CS_PIN) | BIT64(BOARD_RADIO_CS_PIN) | BIT64(BOARD_DISPLAY_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    if (gpio_config(&config) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO init failed");
        return false;
    }

    if (gpio_set_level(BOARD_SDCARD_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set board CS pin high");
        return false;
    }

    if (gpio_set_level(BOARD_RADIO_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set radio CS pin high");
        return false;
    }

    if (gpio_set_level(BOARD_DISPLAY_CS_PIN, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set TFT CS pin high");
        return false;
    }

    return true;
}




void* sdcard_mount(const char* mount_point) {
    ESP_LOGI(TAG, "Mounting %s", mount_point);

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16384,
        .disk_status_check_enable = false
    };

    // Init without card detect (CD) and write protect (WD)
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = BOARD_SDCARD_CS_PIN;
    slot_config.host_id = BOARD_SPI;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // The following value is from T-Deck repo's UnitTest.ino project:
    // https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino
    // Observation: Using this automatically sets the bus to 20MHz
    host.max_freq_khz = 800000U;

    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Mounting failed. Ensure the card is formatted with FAT.");
        } else {
            ESP_LOGE(TAG, "Mounting failed (%s)", esp_err_to_name(ret));
        }
        return NULL;
    }

    td_mountdata_t* data = (td_mountdata_t*)malloc(sizeof(td_mountdata_t));

    data->card = card;
    data->mount_point = mount_point;

    ESP_LOGI(TAG, "Filesystem mounted");
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // TODO: remove this block
    {
        // list the root directory
        DIR *rootdir = opendir(mount_point);
        struct dirent * dirent;
        while((dirent = readdir(rootdir)) != NULL) {
            ESP_LOGI(TAG, "Found %s, id %d, type %d", dirent->d_name, dirent->d_ino, dirent->d_type);
        }
        closedir(rootdir);
    }

    return data;
}

void* sdcard_init_and_mount(const char* mount_point) {
    if (!sdcard_init()) {
        ESP_LOGE(TAG, "Failed to set SPI CS pins high. This is a pre-requisite for mounting.");
        return NULL;
    }
    td_mountdata_t* data = (td_mountdata_t*)sdcard_mount(mount_point);
    if (data == NULL) {
        ESP_LOGE(TAG, "Mount failed for %s", mount_point);
        return NULL;
    }

    sdmmc_card_print_info(stdout, data->card);

    return data;
}

void sdcard_unmount(void* context) {
    td_mountdata_t* data = (td_mountdata_t*)context;
    ESP_LOGI(TAG, "Unmounting %s", data->mount_point);

    assert(data != NULL);
    if (esp_vfs_fat_sdcard_unmount(data->mount_point, data->card) != ESP_OK) {
        ESP_LOGE(TAG, "Unmount failed for %s", data->mount_point);
    }

    free(data);
}

bool sdcard_is_mounted(void* context) {
    td_mountdata_t* data = (td_mountdata_t*)context;
    // TODO: use a semaphore lock since SD, Radio and LCD share the same SPI bus
    //if ( xSemaphoreTake( sd_radio_display_lock, 100 ) ) {
        bool result = (data != NULL) && (sdmmc_get_status(data->card) == ESP_OK);
        // xSemaphoreGive( sd_radio_display_lock );
        return result;
    //} else {
    //    return false;
    //}
}
