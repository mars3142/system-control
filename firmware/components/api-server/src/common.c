#include "common.h"
#include <cJSON.h>
#include <stdbool.h>

#include "api_server.h"
#include "color.h"
#include "message_manager.h"
#include "persistence_manager.h"
#include "simulator.h"
#include <string.h>
#include <time.h>

const char *system_time = NULL;
rgb_t color = {0, 0, 0};

static void on_message_received(const message_t *msg)
{
    if (msg->type == MESSAGE_TYPE_SIMULATION)
    {
        system_time = msg->data.simulation.time;
        color.red = msg->data.simulation.red;
        color.green = msg->data.simulation.green;
        color.blue = msg->data.simulation.blue;

        cJSON *json = create_light_status_json();
        cJSON_AddStringToObject(json, "type", "status");
        char *response = cJSON_PrintUnformatted(json);
        cJSON_Delete(json);
        api_server_ws_broadcast(response);
        free(response);
    }
}

void common_init(void)
{
    message_manager_register_listener(on_message_received);
}

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

    int variant = persistence_manager_get_int(&pm, "light_variant", 3);
    char schema_filename[20];
    snprintf(schema_filename, sizeof(schema_filename), "schema_%02d.csv", variant);
    cJSON_AddStringToObject(json, "schema", schema_filename);

    persistence_manager_deinit(&pm);

    cJSON *c = cJSON_CreateObject();
    cJSON_AddNumberToObject(c, "r", color.red);
    cJSON_AddNumberToObject(c, "g", color.green);
    cJSON_AddNumberToObject(c, "b", color.blue);
    cJSON_AddItemToObject(json, "color", c);

    cJSON_AddStringToObject(json, "clock", system_time);

    return json;
}
