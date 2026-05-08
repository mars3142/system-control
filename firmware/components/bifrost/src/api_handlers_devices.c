#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"
#include "thread_manager.h"

#include <cJSON.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "api_thread";

// ============================================================================
// Thread Devices API
// ============================================================================

esp_err_t api_thread_devices_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/thread/devices");

    size_t                  count;
    const thread_device_t *devs = thread_manager_get_devices(&count);

    cJSON *arr = cJSON_CreateArray();
    for (size_t i = 0; i < count; ++i)
    {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "name",        devs[i].name);
        cJSON_AddStringToObject(obj, "addr",        devs[i].ipv6_addr);
        cJSON_AddBoolToObject(obj,   "has_beacon",  devs[i].has_beacon);
        cJSON_AddBoolToObject(obj,   "has_outdoor", devs[i].has_outdoor);
        cJSON_AddBoolToObject(obj,   "reachable",   devs[i].reachable);
        cJSON_AddBoolToObject(obj,   "beacon_on",   devs[i].beacon_on);
        cJSON_AddBoolToObject(obj,   "outdoor_on",  devs[i].outdoor_on);
        cJSON_AddItemToArray(arr, obj);
    }

    char *json = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    if (!json)
        return send_error_response(req, 500, "Out of memory");

    esp_err_t err = send_json_response(req, json);
    free(json);
    return err;
}

esp_err_t api_thread_devices_add_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/thread/devices");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jname = cJSON_GetObjectItem(json, "name");
    const cJSON *jaddr = cJSON_GetObjectItem(json, "addr");

    if (!is_valid(jname) || !is_valid(jaddr))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing name or addr");
    }

    esp_err_t err = thread_manager_add_device(jname->valuestring, jaddr->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_NO_MEM)
        return send_error_response(req, 507, "Device limit reached");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to add device");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_devices_delete_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/thread/devices");

    char body[128];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jaddr = cJSON_GetObjectItem(json, "addr");
    if (!is_valid(jaddr))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing addr");
    }

    esp_err_t err = thread_manager_remove_device(jaddr->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Device not found");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to remove device");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_devices_set_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/thread/devices/set");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jaddr     = cJSON_GetObjectItem(json, "addr");
    const cJSON *jresource = cJSON_GetObjectItem(json, "resource");
    const cJSON *jon       = cJSON_GetObjectItem(json, "on");

    if (!is_valid(jaddr) || !is_valid(jresource) || !cJSON_IsBool(jon))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing addr, resource or on");
    }

    esp_err_t err = thread_manager_set_resource(jaddr->valuestring, jresource->valuestring,
                                                cJSON_IsTrue(jon));
    cJSON_Delete(json);

    return (err == ESP_OK) ? send_json_response(req, "{\"ok\":true}")
                           : send_error_response(req, 500, "Failed to send command");
}

// ============================================================================
// Thread Groups API
// ============================================================================

esp_err_t api_thread_groups_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/thread/groups");

    size_t                 count;
    const thread_group_t *groups = thread_manager_get_groups(&count);

    cJSON *arr = cJSON_CreateArray();
    for (size_t i = 0; i < count; ++i)
    {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "name", groups[i].name);
        cJSON_AddStringToObject(obj, "addr", groups[i].multicast_addr);

        cJSON *members = cJSON_CreateArray();
        for (uint8_t m = 0; m < groups[i].member_count; ++m)
            cJSON_AddItemToArray(members, cJSON_CreateString(groups[i].member_addrs[m]));
        cJSON_AddItemToObject(obj, "members", members);

        cJSON_AddItemToArray(arr, obj);
    }

    char *json = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    if (!json)
        return send_error_response(req, 500, "Out of memory");

    esp_err_t err = send_json_response(req, json);
    free(json);
    return err;
}

esp_err_t api_thread_groups_add_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/thread/groups");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jname = cJSON_GetObjectItem(json, "name");
    const cJSON *jaddr = cJSON_GetObjectItem(json, "addr");

    if (!is_valid(jname) || !is_valid(jaddr))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing name or addr");
    }

    esp_err_t err = thread_manager_add_group(jname->valuestring, jaddr->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_INVALID_STATE)
        return send_error_response(req, 409, "Group already exists");
    if (err == ESP_ERR_NO_MEM)
        return send_error_response(req, 507, "Group limit reached");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to add group");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_groups_delete_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/thread/groups");

    char body[128];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jaddr = cJSON_GetObjectItem(json, "addr");
    if (!is_valid(jaddr))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing addr");
    }

    esp_err_t err = thread_manager_delete_group(jaddr->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Group not found");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to delete group");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_groups_assign_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/thread/groups/assign");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jdevice = cJSON_GetObjectItem(json, "device_addr");
    const cJSON *jgroup  = cJSON_GetObjectItem(json, "group_addr");

    if (!is_valid(jdevice) || !is_valid(jgroup))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing device_addr or group_addr");
    }

    esp_err_t err = thread_manager_assign_device(jdevice->valuestring, jgroup->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Group not found");
    if (err == ESP_ERR_NO_MEM)
        return send_error_response(req, 507, "Group member limit reached");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to assign device");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_groups_unassign_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/thread/groups/assign");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jdevice = cJSON_GetObjectItem(json, "device_addr");
    const cJSON *jgroup  = cJSON_GetObjectItem(json, "group_addr");

    if (!is_valid(jdevice) || !is_valid(jgroup))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing device_addr or group_addr");
    }

    esp_err_t err = thread_manager_unassign_device(jdevice->valuestring, jgroup->valuestring);
    cJSON_Delete(json);

    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Group or member not found");
    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to unassign device");

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_thread_groups_command_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/thread/groups/command");

    char body[256];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *jaddr     = cJSON_GetObjectItem(json, "addr");
    const cJSON *jresource = cJSON_GetObjectItem(json, "resource");
    const cJSON *jon       = cJSON_GetObjectItem(json, "on");

    if (!is_valid(jaddr) || !is_valid(jresource) || !cJSON_IsBool(jon))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing addr, resource or on");
    }

    esp_err_t err = thread_manager_group_command(jaddr->valuestring, jresource->valuestring,
                                                 cJSON_IsTrue(jon));
    cJSON_Delete(json);

    return (err == ESP_OK) ? send_json_response(req, "{\"ok\":true}")
                           : send_error_response(req, 500, "Failed to send group command");
}

// ============================================================================
// Scenes API
// ============================================================================

esp_err_t api_scenes_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/scenes");
    return send_json_response(req, "[]");
}

esp_err_t api_scenes_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes");

    char body[512];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_scenes_delete_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/scenes");

    char body[128];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    return send_json_response(req, "{\"ok\":true}");
}

esp_err_t api_scenes_activate_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes/activate");

    char body[128];
    int  n = httpd_req_recv(req, body, sizeof(body) - 1);
    if (n <= 0)
        return send_error_response(req, 400, "Empty body");
    body[n] = '\0';

    return send_json_response(req, "{\"ok\":true}");
}
