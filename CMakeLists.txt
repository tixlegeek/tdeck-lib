file(GLOB_RECURSE SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "include" "include/driver"
                    PRIV_REQUIRES esp_psram spi_flash
                    REQUIRES esp_adc driver fatfs vfs)
