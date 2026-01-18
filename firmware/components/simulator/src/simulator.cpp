#include "simulator.h"

#include "color.h"
#include "led_strip_ws2812.h"
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

static const char *TAG = "simulator";
static char *time;

static char *time_to_string(int hhmm)
{
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d Uhr", hhmm / 100, hhmm % 100);
    return buffer;
}

static TaskHandle_t simulation_task_handle = NULL;
static SemaphoreHandle_t simulation_mutex = NULL;

static void ensure_mutex_initialized(void)
{
    if (simulation_mutex == NULL)
    {
        simulation_mutex = xSemaphoreCreateMutex();
    }
}

// The struct is extended with a 'next' pointer to form a linked list.
typedef struct light_item_node_t
{
    char time[5];
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    struct light_item_node_t *next;
} light_item_node_t;

// Global pointers for the head and tail of the list.
static light_item_node_t *head = NULL;
static light_item_node_t *tail = NULL;

// Interpolation mode selection
typedef enum
{
    INTERPOLATION_RGB,
    INTERPOLATION_HSV
} interpolation_mode_t;

// You can change this to test different interpolation methods
static const interpolation_mode_t interpolation_mode = INTERPOLATION_RGB;

char *get_time(void)
{
    return time;
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

esp_err_t add_light_item(const char time[5], uint8_t red, uint8_t green, uint8_t blue, uint8_t white,
                         uint8_t brightness, uint8_t saturation)
{
    // Allocate memory for a new node in PSRAM.
    light_item_node_t *new_node = (light_item_node_t *)heap_caps_malloc(sizeof(light_item_node_t), MALLOC_CAP_SPIRAM);
    if (new_node == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory in PSRAM for new light_item_node_t.");
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

    // Append the new node to the end of the list.
    if (head == NULL)
    {
        // If the list is empty, the new node becomes both head and tail.
        head = new_node;
        tail = new_node;
    }
    else
    {
        // Otherwise, append the new node to the end and update tail.
        tail->next = new_node;
        tail = new_node;
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
    tail = NULL;
    ESP_LOGI(TAG, "Cleaned up all light items.");
}

static void initialize_light_items(void)
{
    cleanup_light_items();
    initialize_storage();

    static char filename[30];
    persistence_manager_t persistence;
    persistence_manager_init(&persistence, "config");
    int variant = persistence_manager_get_int(&persistence, "light_variant", 1);
    snprintf(filename, sizeof(filename), "schema_%02d.csv", variant);
    load_file(filename);
    persistence_manager_deinit(&persistence);

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

static light_item_node_t *find_next_light_item_for_time(int hhmm)
{
    light_item_node_t *current = head;
    light_item_node_t *next_item = NULL;
    int next_time = 9999; // Initialize with a value larger than any possible time

    // First pass: find the soonest time after hhmm
    while (current != NULL)
    {
        int current_time = atoi(current->time);
        if (current_time > hhmm && current_time < next_time)
        {
            next_time = current_time;
            next_item = current;
        }
        current = current->next;
    }

    // If no item is found for the rest of the day, wrap around to the beginning of the next day
    if (next_item == NULL)
    {
        current = head;
        next_time = 9999;
        while (current != NULL)
        {
            int current_time = atoi(current->time);
            if (current_time < next_time)
            {
                next_time = current_time;
                next_item = current;
            }
            current = current->next;
        }
    }
    return next_item;
}

void start_simulate_day(void)
{
    initialize_light_items();

    light_item_node_t *current_item = find_best_light_item_for_time(1200);
    if (current_item != NULL)
    {
        led_strip_update(LED_STATE_DAY,
                         (rgb_t){.red = current_item->red, .green = current_item->green, .blue = current_item->blue});
    }
}

void start_simulate_night(void)
{
    initialize_light_items();

    light_item_node_t *current_item = find_best_light_item_for_time(0);
    if (current_item != NULL)
    {
        led_strip_update(LED_STATE_NIGHT,
                         (rgb_t){.red = current_item->red, .green = current_item->green, .blue = current_item->blue});
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

    initialize_light_items();

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
        light_item_node_t *next_item = find_next_light_item_for_time(hhmm);

        if (current_item != NULL && next_item != NULL)
        {
            int current_item_time_min = (atoi(current_item->time) / 100) * 60 + (atoi(current_item->time) % 100);
            int next_item_time_min = (atoi(next_item->time) / 100) * 60 + (atoi(next_item->time) % 100);

            if (next_item_time_min < current_item_time_min)
            {
                next_item_time_min += total_minutes_in_day;
            }

            int minutes_since_current_item_start = current_minute_of_day - current_item_time_min;
            if (minutes_since_current_item_start < 0)
            {
                minutes_since_current_item_start += total_minutes_in_day;
            }

            int interval_duration = next_item_time_min - current_item_time_min;
            if (interval_duration == 0)
            {
                interval_duration = 1;
            }

            float interpolation_factor = (float)minutes_since_current_item_start / (float)interval_duration;

            // Prepare colors for interpolation
            rgb_t start_rgb = {.red = current_item->red, .green = current_item->green, .blue = current_item->blue};
            rgb_t end_rgb = {.red = next_item->red, .green = next_item->green, .blue = next_item->blue};

            // Use the interpolation function
            rgb_t final_rgb = interpolate_color(start_rgb, end_rgb, interpolation_factor);

            led_strip_update(LED_STATE_SIMULATION, final_rgb);
        }
        else if (current_item != NULL)
        {
            // No next item, just use current
            led_strip_update(
                LED_STATE_SIMULATION,
                (rgb_t){.red = current_item->red, .green = current_item->green, .blue = current_item->blue});
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
        (simulation_config_t *)heap_caps_malloc(sizeof(simulation_config_t), MALLOC_CAP_SPIRAM);
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
            xSemaphoreGive(simulation_mutex);

            // Prüfe ob der Task noch existiert bevor er gelöscht wird
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

void start_simulation(void)
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
