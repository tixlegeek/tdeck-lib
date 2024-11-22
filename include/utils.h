#ifndef _UTILS_H_
#define _UTILS_H_

#include <esp_log.h>
#include <esp_err.h>

#define UNUSED(x) (void)(x)
#define STR_SIZE_TINY 32
#define STR_SIZE_MEDIUM 64
#define STR_SIZE_LARGE 256
#define STR_SIZE_BIG 512

esp_err_t td_ls( const char *directory );

#endif /* end of include guard: _UTILS_H_ */
