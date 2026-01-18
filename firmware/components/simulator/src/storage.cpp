#include "storage.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "simulator.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "storage";

static bool is_spiffs_mounted = false;

void initialize_storage()
{
    if (is_spiffs_mounted)
    {
        return;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    is_spiffs_mounted = true;
}

void load_file(const char *filename)
{
    ESP_LOGI(TAG, "Loading file: %s", filename);
    int line_count = 0;
    char **lines = read_lines_filtered(filename, &line_count);
    uint8_t line_number = 0;
    for (int i = 0; i < line_count; ++i)
    {
        char time[10] = {0};
        int red, green, blue, white, brightness, saturation;
        int items_scanned =
            sscanf(lines[i], "%d,%d,%d,%d,%d,%d", &red, &green, &blue, &white, &brightness, &saturation);
        if (items_scanned == 6)
        {
            int total_minutes = line_number * 30;
            int hours = total_minutes / 60;
            int minutes = total_minutes % 60;
            snprintf(time, sizeof(time), "%02d%02d", hours, minutes);
            add_light_item(time, red, green, blue, white, brightness, saturation);
            line_number++;
        }
        else
        {
            ESP_LOGW(TAG, "Could not parse line: %s", lines[i]);
        }
    }
    free_lines(lines, line_count);
    ESP_LOGI(TAG, "Finished loading file. Loaded %d entries.", line_number);
}

char **read_lines_filtered(const char *filename, int *out_count)
{
    char fullpath[128];
    snprintf(fullpath, sizeof(fullpath), "/spiffs/%s", filename[0] == '/' ? filename + 1 : filename);
    FILE *f = fopen(fullpath, "r");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", fullpath);
        *out_count = 0;
        return NULL;
    }

    size_t capacity = 16;
    size_t count = 0;
    char **lines = (char **)malloc(capacity * sizeof(char *));
    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        // Zeilenumbruch entfernen
        char *pos = strchr(line, '\n');
        if (pos)
            *pos = '\0';
        // Trim vorne
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
            trimmed++;
        // Leere oder Kommentarzeile überspringen
        if (*trimmed == '\0' || *trimmed == '#')
            continue;
        // Trim hinten
        size_t len = strlen(trimmed);
        while (len > 0 && (trimmed[len - 1] == ' ' || trimmed[len - 1] == '\t'))
            trimmed[--len] = '\0';
        // Kopieren
        if (count >= capacity)
        {
            capacity *= 2;
            lines = (char **)realloc(lines, capacity * sizeof(char *));
        }
        lines[count++] = strdup(trimmed);
    }
    fclose(f);
    *out_count = (int)count;
    return lines;
}

void free_lines(char **lines, int count)
{
    for (int i = 0; i < count; ++i)
        free(lines[i]);
    free(lines);
}
