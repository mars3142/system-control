#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define THREAD_MAX_DEVICES       16
#define THREAD_MAX_GROUPS         8
#define THREAD_GROUP_MAX_MEMBERS  8

    typedef struct
    {
        char name[32];
        char ipv6_addr[46];
        bool has_beacon;
        bool has_outdoor;
        bool reachable;
        bool beacon_on;   // current state, updated via CoAP Observe (RFC 7641)
        bool outdoor_on;  // current state, updated via CoAP Observe (RFC 7641)
    } thread_device_t;

    typedef struct
    {
        char    name[32];
        char    multicast_addr[46];
        char    member_addrs[THREAD_GROUP_MAX_MEMBERS][46];
        uint8_t member_count;
    } thread_group_t;

    // Callback fired on async events; json_event is valid only for the duration of the call.
    typedef void (*thread_event_cb_t)(const char *json_event);

    // Initialize Thread coordinator: forms network as Leader, starts Commissioner,
    // and listens for CoAP /announce messages from H2 devices.
    esp_err_t thread_manager_init(thread_event_cb_t event_cb);

    // Device management
    const thread_device_t *thread_manager_get_devices(size_t *count);
    esp_err_t              thread_manager_add_device(const char *name, const char *ipv6_addr);
    esp_err_t              thread_manager_remove_device(const char *ipv6_addr);

    // Send CoAP PUT to a device. resource = "beacon" | "outdoor". Fire-and-forget.
    esp_err_t thread_manager_set_resource(const char *ipv6_addr, const char *resource, bool on);

    // Group management
    const thread_group_t *thread_manager_get_groups(size_t *count);
    esp_err_t             thread_manager_add_group(const char *name, const char *multicast_addr);
    esp_err_t             thread_manager_delete_group(const char *multicast_addr);
    esp_err_t             thread_manager_assign_device(const char *device_addr, const char *multicast_addr);
    esp_err_t             thread_manager_unassign_device(const char *device_addr, const char *multicast_addr);

    // Send CoAP PUT to a multicast address (group command). Fire-and-forget.
    esp_err_t thread_manager_group_command(const char *multicast_addr, const char *resource, bool on);

#ifdef __cplusplus
}
#endif
