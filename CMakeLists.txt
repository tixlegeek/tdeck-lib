file(GLOB_RECURSE SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "include" "include/driver"
                    REQUIRES esp_adc driver fatfs vfs)
