#include "common.h"
#include <cJSON.h>
#include <stdbool.h>

#include "persistence_manager.h"

// Gibt ein cJSON-Objekt with dem aktuellen Lichtstatus zurück
cJSON *create_light_status_json(void)
{
    persistence_manager_t pm;
    persistence_manager_init(&pm, "config");
    cJSON *json = cJSON_CreateObject();

    bool light_active = persistence_manager_get_bool(&pm, "light_active", false);
    cJSON_AddBoolToObject(json, "on", light_active);

    cJSON_AddBoolToObject(json, "thunder", false);

    int mode = persistence_manager_get_int(&pm, "light_mode", 0);
    const char *mode_str = "simulation";
    if (mode == 1)
    {
        mode_str = "day";
    }
    else if (mode == 2)
    {
        mode_str = "night";
    }
    cJSON_AddStringToObject(json, "mode", mode_str);

    cJSON_AddStringToObject(json, "schema", "schema_03.csv");
    cJSON *color = cJSON_CreateObject();
    cJSON_AddNumberToObject(color, "r", 255);
    cJSON_AddNumberToObject(color, "g", 240);
    cJSON_AddNumberToObject(color, "b", 220);
    cJSON_AddItemToObject(json, "color", color);

    persistence_manager_deinit(&pm);

    return json;
}
