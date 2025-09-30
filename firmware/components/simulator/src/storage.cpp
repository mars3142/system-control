#include "storage.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "simulator.h"
#include <errno.h>
#include <stdio.h>

static const char *TAG = "storage";

void initialize_storage()
{
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
}

void load_file(const char *filename)
{
    ESP_LOGI(TAG, "Loading file: %s", filename);
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char line[128];
    uint8_t line_number = 0;
    while (fgets(line, sizeof(line), f))
    {
        char *pos = strchr(line, '\n');
        if (pos)
        {
            *pos = '\0';
        }

        if (strlen(line) == 0)
        {
            continue;
        }

        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
        {
            trimmed++;
        }
        if (*trimmed == '#' || *trimmed == '\0')
        {
            continue;
        }

        char time[10] = {0};
        int red, green, blue, white, brightness, saturation;

        int items_scanned = sscanf(line, "%d,%d,%d,%d,%d,%d", &red, &green, &blue, &white, &brightness, &saturation);
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
            ESP_LOGW(TAG, "Could not parse line: %s", line);
        }
    }

    fclose(f);
    ESP_LOGI(TAG, "Finished loading file. Loaded %d entries.", line_number);
}
