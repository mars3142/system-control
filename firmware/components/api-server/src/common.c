#include "common.h"
#include <cJSON.h>
#include <stdbool.h>

// Gibt ein cJSON-Objekt with dem aktuellen Lichtstatus zurück
cJSON *create_light_status_json(void)
{
    cJSON *json = cJSON_CreateObject();
    // TODO: Echte Werte einfügen, aktuell Dummy-Daten
    cJSON_AddBoolToObject(json, "on", false);
    cJSON_AddBoolToObject(json, "thunder", false);
    cJSON_AddStringToObject(json, "mode", "day");
    cJSON_AddStringToObject(json, "schema", "schema_03.csv");
    cJSON *color = cJSON_CreateObject();
    cJSON_AddNumberToObject(color, "r", 255);
    cJSON_AddNumberToObject(color, "g", 240);
    cJSON_AddNumberToObject(color, "b", 220);
    cJSON_AddItemToObject(json, "color", color);
    return json;
}
