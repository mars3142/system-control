#include "simulator.h"
#include "color.h"
#include "led_strip_ws2812.h"
#include "message_manager.h"
#include "persistence_manager.h"
#include "storage.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Type definitions
typedef struct light_item_node_t
{
    char time[4];
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    struct light_item_node_t *next;
} light_item_node_t;

typedef enum
{
    INTERPOLATION_RGB,
    INTERPOLATION_HSV
} interpolation_mode_t;

// Constants and global variables
static const char *TAG = "simulator";
static char *time = NULL;
static TaskHandle_t simulation_task_handle = NULL;
static SemaphoreHandle_t simulation_mutex = NULL;
static light_item_node_t *head = NULL;
static bool schema_loaded = false;
static int loaded_variant = -1;
static const interpolation_mode_t interpolation_mode = INTERPOLATION_RGB;

// Helper function: converts hhmm format to minutes of the day
static int hhmm_to_minutes(const char time[5])
{
    int t = atoi(time);
    return (t / 100) * 60 + (t % 100);
}

// Helper function: converts int hhmm to string
static char *time_to_string(int hhmm)
{
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hhmm / 100, hhmm % 100);
    return buffer;
}

// Helper function: ensures mutex is initialized
static void ensure_mutex_initialized(void)
{
    if (simulation_mutex == NULL)
    {
        simulation_mutex = xSemaphoreCreateMutex();
    }
}

// Main interpolation function that selects the appropriate method
static rgb_t interpolate_color(rgb_t start, rgb_t end, float factor)
{
    switch (interpolation_mode)
    {
    case INTERPOLATION_RGB:
        return interpolate_color_rgb(start, end, factor);
    case INTERPOLATION_HSV:
    default:
        return interpolate_color_hsv(start, end, factor);
    }
}

// Linked list management
esp_err_t add_light_item(const char time[5], uint8_t red, uint8_t green, uint8_t blue, uint8_t white,
                         uint8_t brightness, uint8_t saturation)
{
    // Allocate memory for new node in PSRAM.
    light_item_node_t *new_node = (light_item_node_t *)heap_caps_malloc(sizeof(light_item_node_t), MALLOC_CAP_DEFAULT);
    if (new_node == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for new light_item_node_t.");
        return ESP_FAIL;
    }

    rgb_t color = {.red = red, .green = green, .blue = blue};

    if (saturation < 255)
    {
        hsv_t hsv = rgb_to_hsv(color);
        hsv.s = hsv.s * (saturation / 255.0f);
        // color = hsv_to_rgb(hsv);
    }

    float brightness_factor = brightness / 255.0f;

    memcpy(new_node->time, time, sizeof(new_node->time));
    new_node->red = (uint8_t)(color.red * brightness_factor);
    new_node->green = (uint8_t)(color.green * brightness_factor);
    new_node->blue = (uint8_t)(color.blue * brightness_factor);
    new_node->next = NULL;

    // Insert sorted: find the correct position
    if (head == NULL || hhmm_to_minutes(new_node->time) < hhmm_to_minutes(head->time))
    {
        // New head
        new_node->next = head;
        head = new_node;
    }
    else
    {
        light_item_node_t *prev = head;
        while (prev->next != NULL && hhmm_to_minutes(prev->next->time) < hhmm_to_minutes(new_node->time))
        {
            prev = prev->next;
        }
        new_node->next = prev->next;
        prev->next = new_node;
    }

    return ESP_OK;
}

void cleanup_light_items(void)
{
    light_item_node_t *current = head;
    light_item_node_t *next_node;

    while (current != NULL)
    {
        next_node = current->next;
        heap_caps_free(current);
        current = next_node;
    }

    head = NULL;
    schema_loaded = false;
    loaded_variant = -1;
    ESP_LOGI(TAG, "Cleaned up all light items.");
}

static void initialize_light_items(bool force_reload)
{
    static char filename[30];
    persistence_manager_t persistence;
    persistence_manager_init(&persistence, "config");
    int variant = persistence_manager_get_int(&persistence, "light_variant", 1);
    persistence_manager_deinit(&persistence);

    bool variant_changed = (loaded_variant != variant);
    bool needs_reload = force_reload || !schema_loaded || variant_changed;

    if (needs_reload)
    {
        cleanup_light_items();
        initialize_storage();

        snprintf(filename, sizeof(filename), "schema_%02d.csv", variant);
        load_file(filename);
        schema_loaded = true;
        loaded_variant = variant;
        ESP_LOGI(TAG, "Schema loaded (variant=%d, force_reload=%s)", variant, force_reload ? "true" : "false");
    }
    else
    {
        ESP_LOGD(TAG, "Schema reload skipped (variant=%d unchanged)", variant);
    }

    // The list is now sorted because add_light_item inserts sorted

    if (head == NULL)
    {
        ESP_LOGW(TAG, "Light schedule is empty. Simulation will not run.");
        vTaskDelete(NULL);
        return;
    }
}

static light_item_node_t *find_best_light_item_for_time(int hhmm)
{
    light_item_node_t *best_item = NULL;
    light_item_node_t *current = head;
    int best_time = -1;

    while (current != NULL)
    {
        int current_time = atoi(current->time);
        if (current_time <= hhmm && current_time > best_time)
        {
            best_time = current_time;
            best_item = current;
        }
        current = current->next;
    }

    if (best_item == NULL)
    {
        // If no item is found for the given time (e.g., before the first item of the day),
        // find the last item of the previous day.
        best_time = -1;
        current = head;
        while (current != NULL)
        {
            int current_time = atoi(current->time);
            if (current_time > best_time)
            {
                best_time = current_time;
                best_item = current;
            }
            current = current->next;
        }
    }

    return best_item;
}

// Messaging
static void send_simulation_message(const char *time, rgb_t color)
{
    message_t msg = {};
    msg.type = MESSAGE_TYPE_SIMULATION;
    strncpy(msg.data.simulation.time, time, sizeof(msg.data.simulation.time) - 1);
    msg.data.simulation.time[sizeof(msg.data.simulation.time) - 1] = '\0';
    msg.data.simulation.red = color.red;
    msg.data.simulation.green = color.green;
    msg.data.simulation.blue = color.blue;
    message_manager_post(&msg, pdMS_TO_TICKS(100));
}

// Public API
char *get_time(void)
{
    return time;
}

void start_simulate_day(void)
{
    initialize_light_items(false);

    light_item_node_t *current_item = find_best_light_item_for_time(1200);
    if (current_item != NULL)
    {
        rgb_t color = {.red = current_item->red, .green = current_item->green, .blue = current_item->blue};
        led_strip_update(LED_STATE_DAY, color);
        send_simulation_message("12:00", color);
    }
}

void start_simulate_night(void)
{
    initialize_light_items(false);

    light_item_node_t *current_item = find_best_light_item_for_time(0);
    if (current_item != NULL)
    {
        rgb_t color = {.red = current_item->red, .green = current_item->green, .blue = current_item->blue};
        led_strip_update(LED_STATE_NIGHT, color);
        send_simulation_message("00:00", color);
    }
}

void simulate_cycle(void *args)
{
    simulation_config_t *config = (simulation_config_t *)args;
    int cycle_duration_minutes = config->cycle_duration_minutes;
    heap_caps_free(config);

    if (cycle_duration_minutes <= 0)
    {
        ESP_LOGE(TAG, "Invalid cycle duration: %d minutes. Must be positive.", cycle_duration_minutes);
        if (simulation_mutex != NULL && xSemaphoreTake(simulation_mutex, portMAX_DELAY) == pdTRUE)
        {
            simulation_task_handle = NULL;
            xSemaphoreGive(simulation_mutex);
        }
        vTaskDelete(NULL);
        return;
    }

    initialize_light_items(false);

    const int total_minutes_in_day = 24 * 60;
    long delay_ms = (long)cycle_duration_minutes * 60 * 1000 / total_minutes_in_day;
    ESP_LOGI(TAG, "Starting simulation of a 24h cycle over %d minutes. Each simulated minute will take %ld ms.",
             cycle_duration_minutes, delay_ms);

    int current_minute_of_day = 0;

    while (1)
    {
        int hours = current_minute_of_day / 60;
        int minutes = current_minute_of_day % 60;
        int hhmm = hours * 100 + minutes;
        time = time_to_string(hhmm);

        light_item_node_t *current_item = find_best_light_item_for_time(hhmm);
        light_item_node_t *next_item = NULL;

        if (current_item != NULL)
        {
            rgb_t color = {0, 0, 0};

            // Cyclic interpolation: if current_item is the tail element, set next_item to head
            if (current_item->next == NULL && head != NULL)
            {
                next_item = head;
            }
            else
            {
                next_item = current_item->next;
            }

            if (next_item != NULL)
            {
                int current_minutes = hhmm_to_minutes(current_item->time);
                int next_minutes = hhmm_to_minutes(next_item->time);

                // Cyclic transition: if next_minutes < current_minutes, add day length
                if (next_minutes < current_minutes)
                {
                    next_minutes += total_minutes_in_day;
                }

                int minutes_since_current = current_minute_of_day - current_minutes;
                if (minutes_since_current < 0)
                {
                    minutes_since_current += total_minutes_in_day;
                }

                int interval = next_minutes - current_minutes;
                if (interval == 0)
                {
                    interval = 1;
                }

                float factor = (float)minutes_since_current / (float)interval;

                rgb_t start_rgb = {.red = current_item->red, .green = current_item->green, .blue = current_item->blue};
                rgb_t end_rgb = {.red = next_item->red, .green = next_item->green, .blue = next_item->blue};

                color = interpolate_color(start_rgb, end_rgb, factor);
                led_strip_update(LED_STATE_SIMULATION, color);
            }
            else
            {
                color = (rgb_t){.red = current_item->red, .green = current_item->green, .blue = current_item->blue};
                led_strip_update(LED_STATE_SIMULATION, color);
            }
            send_simulation_message(time, color);
        }

        vTaskDelay(pdMS_TO_TICKS(delay_ms));

        current_minute_of_day++;
        if (current_minute_of_day >= total_minutes_in_day)
        {
            current_minute_of_day = 0;
            ESP_LOGI(TAG, "Simulation cycle restarting.");
        }
    }
}

void start_simulation_task(void)
{
    stop_simulation_task();

    simulation_config_t *config =
        (simulation_config_t *)heap_caps_malloc(sizeof(simulation_config_t), MALLOC_CAP_DEFAULT);
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for simulation config.");
        return;
    }

    config->cycle_duration_minutes = 15;

    if (xTaskCreatePinnedToCore(simulate_cycle, "simulate_cycle", 4096, (void *)config, tskIDLE_PRIORITY + 1,
                                &simulation_task_handle, CONFIG_FREERTOS_NUMBER_OF_CORES - 1) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create simulation task.");
        heap_caps_free(config);
    }
}

void stop_simulation_task(void)
{
    ensure_mutex_initialized();

    if (xSemaphoreTake(simulation_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (simulation_task_handle != NULL)
        {
            TaskHandle_t handle_to_delete = simulation_task_handle;
            simulation_task_handle = NULL;
            time = NULL;
            xSemaphoreGive(simulation_mutex);

            // Check if the task still exists before deleting it
            eTaskState state = eTaskGetState(handle_to_delete);
            if (state != eDeleted && state != eInvalid)
            {
                vTaskDelete(handle_to_delete);
            }
        }
        else
        {
            xSemaphoreGive(simulation_mutex);
        }
    }
}

void start_simulation_with_reload(bool force_reload)
{
    stop_simulation_task();

    persistence_manager_t persistence;
    persistence_manager_init(&persistence, "config");
    if (persistence_manager_get_bool(&persistence, "light_active", false))
    {
        int mode = persistence_manager_get_int(&persistence, "light_mode", 0);
        switch (mode)
        {
        case 0: // Simulation mode
            if (force_reload)
            {
                initialize_light_items(true);
            }
            start_simulation_task();
            break;
        case 1: // Day mode
            start_simulate_day();
            break;
        case 2: // Night mode
            start_simulate_night();
            break;
        default:
            ESP_LOGW(TAG, "Unknown light mode: %d", mode);
            break;
        }
    }
    else
    {
        led_strip_update(LED_STATE_OFF, rgb_t{});
    }
    persistence_manager_deinit(&persistence);
}

void start_simulation(void)
{
    start_simulation_with_reload(true);
}
