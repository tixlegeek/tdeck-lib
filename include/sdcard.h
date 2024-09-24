#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>
#include <utils.h>

#include <dirent.h>

typedef struct td_sdcard_t {
    bool initialized;
    const char mount_point[STR_SIZE_MEDIUM];
    spi_device_handle_t dev;
    sdmmc_card_t* card;
    bool mounted;
} td_sdcard_t;

esp_err_t td_sdcard_init(void *ctx, const char *mount_point);
esp_err_t sdcard_mount(void *ctx, const char *mount_point);
esp_err_t sdcard_unmount(void *ctx);
esp_err_t sdcard_is_mounted(void *ctx);



#endif
