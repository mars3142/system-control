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
        .base_path = "/spiffs",         // Der Basispfad, unter dem das Dateisystem gemountet wird
        .partition_label = NULL,        // NULL, um die erste gefundene SPIFFS-Partition zu verwenden
        .max_files = 5,                 // Maximale Anzahl gleichzeitig geöffneter Dateien
        .format_if_mount_failed = false // Partition formatieren, wenn das Mounten fehlschlägt
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
        return; // Oder entsprechende Fehlerbehandlung
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

    char line[128]; // Puffer für eine Zeile, vergrößert für mehr Sicherheit
    while (fgets(line, sizeof(line), f))
    {
        // Entferne möglichen Zeilenumbruch am Ende
        char *pos = strchr(line, '\n');
        if (pos)
        {
            *pos = '\0';
        }

        char time[5] = {0}; // 4 Zeichen + Nullterminator
        int red, green, blue;

        // Parse die Zeile im Format "HHMM,R,G,B"
        int items_scanned = sscanf(line, "%4[^,],%d,%d,%d", time, &red, &green, &blue);
        if (items_scanned == 4)
        {
            add_light_item(time, (uint8_t)red, (uint8_t)green, (uint8_t)blue);
        }
        else
        {
            ESP_LOGW(TAG, "Could not parse line: %s", line);
        }
    }

    fclose(f);
    ESP_LOGI(TAG, "Finished loading file.");
}