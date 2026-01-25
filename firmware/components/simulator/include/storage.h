#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void initialize_storage();
    void load_file(const char *filename);
    char **read_lines_filtered(const char *filename, int *out_count);
    void free_lines(char **lines, int count);
    /**
     * Write an array of lines to a file (CSV or other text).
     * @param filename File name (without /spiffs/)
     * @param lines Array of lines (null-terminated strings)
     * @param count Number of lines
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t write_lines(const char *filename, char **lines, int count);
#ifdef __cplusplus
}
#endif
