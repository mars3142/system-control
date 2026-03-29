#include "iris/iris_internal.h"

#if defined(CONFIG_IRIS_ENABLED)

static const char *TAG = "Iris";

void spiffs_save(void)
{
    FILE *f = fopen(IRIS_STORE_PATH, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for write", IRIS_STORE_PATH);
        return;
    }

    iris_store_header_t hdr = {
        .magic   = IRIS_STORE_MAGIC,
        .version = IRIS_STORE_VERSION,
        .count   = (uint16_t)s_paired_count,
    };
    fwrite(&hdr, sizeof(hdr), 1, f);

    for (int i = 0; i < s_paired_count; i++) {
        fwrite(&s_paired[i].p, sizeof(iris_device_persisted_t), 1, f);
    }
    fclose(f);
    ESP_LOGD(TAG, "Saved %d device(s) to SPIFFS", s_paired_count);
}

void spiffs_load(void)
{
    FILE *f = fopen(IRIS_STORE_PATH, "rb");
    if (!f) {
        ESP_LOGI(TAG, "No device store found — starting fresh");
        return;
    }

    iris_store_header_t hdr = {};
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 || hdr.magic != IRIS_STORE_MAGIC) {
        ESP_LOGW(TAG, "Invalid or corrupt device store — ignoring");
        fclose(f);
        return;
    }
    if (hdr.version != IRIS_STORE_VERSION) {
        ESP_LOGW(TAG, "Unsupported store version %u — ignoring", hdr.version);
        fclose(f);
        return;
    }

    int count = hdr.count;
    if (count > CONFIG_IRIS_MAX_DEVICES) {
        ESP_LOGW(TAG, "Store has %d devices, capping at %d", count, CONFIG_IRIS_MAX_DEVICES);
        count = CONFIG_IRIS_MAX_DEVICES;
    }

    s_paired_count = 0;
    for (int i = 0; i < count; i++) {
        iris_device_persisted_t p = {};
        if (fread(&p, sizeof(p), 1, f) != 1)
            break;
        s_paired[s_paired_count].p            = p;
        s_paired[s_paired_count].online       = false;
        s_paired[s_paired_count].failed_polls = 0;
        s_paired_count++;
    }
    fclose(f);
    ESP_LOGI(TAG, "Loaded %d paired device(s) from SPIFFS", s_paired_count);
}

#endif /* CONFIG_IRIS_ENABLED */
