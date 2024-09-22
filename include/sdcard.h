#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include <dirent.h>

typedef struct td_mountdata_t {
    const char* mount_point;
    sdmmc_card_t* card;
} td_mountdata_t;


#define SDCARD_MOUNT_POINT "/sdcard"

bool sdcard_init();
void* sdcard_mount(const char* mount_point);
void* sdcard_init_and_mount(const char* mount_point);
void sdcard_unmount(void* context);
bool sdcard_is_mounted(void* context);

esp_err_t td_sdcard_init(void *ctx);


#endif
