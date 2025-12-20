#include "ble/ble_connection.h"

#include <esp_log.h>
#include <host/ble_gap.h>
#include <host/ble_hs.h>
#include <host/ble_sm.h>
#include <host/ble_store.h>
#include <string.h>

static const char *TAG = "ble_connection";

static uint16_t g_conn_handle;
static uint16_t g_char_val_handle; // Handle der Characteristic, die du lesen willst
static bool g_bonding_in_progress = false;

const char *ble_error_to_string(int status)
{
    switch (status)
    {
    case 0:
        return "Success";
    case BLE_HS_EDONE:
        return "Operation complete";
    case BLE_HS_EALREADY:
        return "Operation already in progress";
    case BLE_HS_EINVAL:
        return "Invalid argument";
    case BLE_HS_EMSGSIZE:
        return "Message too large";
    case BLE_HS_ENOENT:
        return "No entry found";
    case BLE_HS_ENOMEM:
        return "Out of memory";
    case BLE_HS_ENOTCONN:
        return "Not connected";
    case BLE_HS_ENOTSUP:
        return "Not supported";
    case BLE_HS_EAPP:
        return "Application error";
    case BLE_HS_EBADDATA:
        return "Bad data";
    case BLE_HS_EOS:
        return "OS error";
    case BLE_HS_ECONTROLLER:
        return "Controller error";
    case BLE_HS_ETIMEOUT:
        return "Timeout";
    case BLE_HS_EBUSY:
        return "Busy";
    case BLE_HS_EREJECT:
        return "Rejected";
    case BLE_HS_EUNKNOWN:
        return "Unknown error";
    case BLE_HS_EROLE:
        return "Role error";
    case BLE_HS_ETIMEOUT_HCI:
        return "HCI timeout";
    case BLE_HS_ENOMEM_EVT:
        return "No memory for event";
    case BLE_HS_ENOADDR:
        return "No address";
    case BLE_HS_ENOTSYNCED:
        return "Not synchronized";
    case BLE_HS_EAUTHEN:
        return "Authentication failed";
    case BLE_HS_EAUTHOR:
        return "Authorization failed";
    case BLE_HS_EENCRYPT:
        return "Encryption failed";
    case BLE_HS_EENCRYPT_KEY_SZ:
        return "Encryption key size";
    case BLE_HS_ESTORE_CAP:
        return "Storage capacity exceeded";
    case BLE_HS_ESTORE_FAIL:
        return "Storage failure";
    default:
        // ATT-Fehler prüfen
        if ((status & 0x100) == 0x100)
        {
            return "ATT error";
        }
        return "Unknown error";
    }
}

static void ble_sm_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_PASSKEY_ACTION:
        ESP_LOGI(TAG, "Passkey action required");
        // Hier können Sie Passkey-Aktionen implementieren
        // z.B. Display passkey, Input passkey, etc.
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(TAG, "Encryption change: status=%d", event->enc_change.status);
        if (event->enc_change.status == 0)
        {
            ESP_LOGI(TAG, "Encryption established successfully");
            g_bonding_in_progress = false;
        }
        break;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        ESP_LOGI(TAG, "Repeat pairing");
        break;

    default:
        break;
    }
}

static bool is_device_bonded(const ble_addr_t *addr)
{
    struct ble_store_value_sec sec_value;
    struct ble_store_key_sec sec_key = {0};

    sec_key.peer_addr = *addr;
    sec_key.idx = 0;

    int rc = ble_store_read_peer_sec(&sec_key, &sec_value);
    return (rc == 0);
}

static void initiate_bonding(uint16_t conn_handle)
{
    if (!g_bonding_in_progress)
    {
        g_bonding_in_progress = true;
        ESP_LOGI(TAG, "Initiating bonding for connection %d", conn_handle);

        // Starte Security/Bonding Prozess
        int rc = ble_gap_security_initiate(conn_handle);
        if (rc != 0)
        {
            ESP_LOGE(TAG, "Failed to initiate security: %s", ble_error_to_string(rc));
            g_bonding_in_progress = false;
        }
    }
}

static int gattc_svcs_callback(uint16_t conn_handle, const struct ble_gatt_error *error,
                               const struct ble_gatt_svc *service, void *arg)
{
    if (error->status != 0)
    {
        ESP_LOGE(TAG, "Error discovering service: %s", ble_error_to_string(error->status));
        return 0;
    }

    char uuid_str[37]; // Maximale Länge für 128-bit UUID
    ble_uuid_to_str(&service->uuid.u, uuid_str);
    ESP_LOGI(TAG, "Discovered service: %s", uuid_str);

    return 0;
}

static int gattc_char_callback(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr,
                               void *arg)
{
    if (error->status != 0)
    {
        ESP_LOGE(TAG, "Error discovering characteristic: %d", error->status);
        return 0;
    }

    g_char_val_handle = chr->val_handle;
    read_characteristic(chr->val_handle);
    return 0;
}

// Callback für GATT-Events
static int gattc_event_callback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                const struct ble_gatt_svc *service, void *arg)
{
    if (error->status != 0)
    {
        ESP_LOGE(TAG, "Error discovering service: %d", error->status);
        return 0;
    }

    ble_gattc_disc_all_svcs(conn_handle, gattc_svcs_callback, NULL);
    // ble_gattc_disc_all_chrs(conn_handle, service->start_handle, service->end_handle, gattc_char_callback, NULL);
    return 0;
}

static int gattc_read_callback(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr,
                               void *arg)
{
    if (error->status == 0)
    {
        ESP_LOGI(TAG, "Wert gelesen %d, Länge: %d", attr->handle, attr->om->om_len);
        ESP_LOG_BUFFER_HEX("READ_DATA", attr->om->om_data, attr->om->om_len);
    }
    else
    {
        ESP_LOGE(TAG, "Lesefehler, Status: %d", error->status);
    }
    return 0;
}

static int ble_gap_event_handler(struct ble_gap_event *event, void *arg)
{
    device_info_t *device = (device_info_t *)arg;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0)
        {
            g_conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Connected; conn_handle=%d", g_conn_handle);

            // Prüfe ob Device bereits gebondet ist
            if (is_device_bonded(&device->addr))
            {
                ESP_LOGI(TAG, "Device already bonded, using existing bond");
                // Bei gebondetem Device kann direkt mit Service Discovery begonnen werden
                ble_gattc_disc_all_svcs(g_conn_handle, gattc_event_callback, NULL);
            }
            else
            {
                ESP_LOGI(TAG, "Device not bonded, initiating bonding");
                // Starte Bonding-Prozess
                initiate_bonding(g_conn_handle);
                // Service Discovery wird nach erfolgreichem Bonding gestartet
            }
        }
        else
        {
            ESP_LOGE(TAG, "Connection failed; status=%d", event->connect.status);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        g_conn_handle = 0;
        g_bonding_in_progress = false;
        ESP_LOGI(TAG, "Disconnected; reason=%d", event->disconnect.reason);
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(TAG, "Connection updated; status=%d", event->conn_update.status);
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        ESP_LOGI(TAG, "Encryption change: status=%d", event->enc_change.status);
        if (event->enc_change.status == 0)
        {
            ESP_LOGI(TAG, "Encryption established, bonding complete");
            g_bonding_in_progress = false;
            // Nach erfolgreichem Bonding: Service Discovery starten
            ble_gattc_disc_all_svcs(g_conn_handle, gattc_event_callback, NULL);
        }
        else
        {
            ESP_LOGE(TAG, "Encryption failed: %s", ble_error_to_string(event->enc_change.status));
            g_bonding_in_progress = false;
        }
        break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        ESP_LOGI(TAG, "Passkey action event");
        // Implementieren Sie hier die Passkey-Behandlung
        // z.B. einen festen Passkey eingeben:
        struct ble_sm_io pkey = {0};
        pkey.action = BLE_SM_IOACT_INPUT;
        pkey.passkey = 100779;
        ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        break;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        ESP_LOGI(TAG, "Device requests repeat pairing");

        // Hole die Peer-Adresse aus der Verbindung
        struct ble_gap_conn_desc conn_desc;
        int rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &conn_desc);
        if (rc == 0)
        {
            // Lösche alte Bonding-Info
            ble_clear_bond(&conn_desc.peer_ota_addr);
        }

        // Erlaube erneutes Pairing
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    default:
        break;
    }
    return 0;
}

void ble_connect(device_info_t *device)
{
    struct ble_gap_conn_params conn_params = {
        .scan_itvl = 0x0010,
        .scan_window = 0x0010,
        .itvl_min = BLE_GAP_INITIAL_CONN_ITVL_MIN,
        .itvl_max = BLE_GAP_INITIAL_CONN_ITVL_MAX,
        .latency = BLE_GAP_INITIAL_CONN_LATENCY,
        .supervision_timeout = BLE_GAP_INITIAL_SUPERVISION_TIMEOUT,
        .min_ce_len = BLE_GAP_INITIAL_CONN_MIN_CE_LEN,
        .max_ce_len = BLE_GAP_INITIAL_CONN_MAX_CE_LEN,
    };

    // Prüfe ob Device bereits gebondet ist
    if (is_device_bonded(&device->addr))
    {
        ESP_LOGI(TAG, "Connecting to bonded device");
    }
    else
    {
        ESP_LOGI(TAG, "Connecting to new device (will bond after connection)");
    }

    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &device->addr, 30000, &conn_params, ble_gap_event_handler, device);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error initiating connection: %s", ble_error_to_string(rc));
    }
}

void ble_clear_bonds(void)
{
    ESP_LOGI(TAG, "Clearing all bonds");

    int rc = ble_store_clear();
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to clear bond storage: %s", ble_error_to_string(rc));
    }
    else
    {
        ESP_LOGI(TAG, "All bonds cleared successfully");
    }
}

// Funktion zum Löschen der Bonding-Info eines bestimmten Devices
void ble_clear_bond(const ble_addr_t *addr)
{
    ESP_LOGI(TAG, "Clearing bond for specific device");

    int rc = ble_store_util_delete_peer(addr);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to delete peer: %s", ble_error_to_string(rc));
    }
    else
    {
        ESP_LOGI(TAG, "Peer deleted successfully");
    }
}

void read_characteristic(uint16_t char_val_handle)
{
    if (char_val_handle != 0 && g_conn_handle != 0)
    {
        int rc = ble_gattc_read(g_conn_handle, char_val_handle, gattc_read_callback, NULL);
        if (rc != 0)
        {
            ESP_LOGE(TAG, "Error reading characteristic: %d", rc);
        }
    }
}
