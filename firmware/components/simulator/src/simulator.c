#include "simulator.h"

#include "color.h"
#include "led_strip_ws2812.h"
#include "storage.h"
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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

char *get_time(void)
{
    return time;
}

esp_err_t add_light_item(const char time[5], uint8_t red, uint8_t green, uint8_t blue)
{
    // Allocate memory for a new node in PSRAM.
    light_item_node_t *new_node = (light_item_node_t *)heap_caps_malloc(sizeof(light_item_node_t), MALLOC_CAP_SPIRAM);
    if (new_node == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory in PSRAM for new light_item_node_t.");
        return ESP_FAIL;
    }

    // Initialize the data of the new node.
    memcpy(new_node->time, time, sizeof(new_node->time));
    new_node->red = red;
    new_node->green = green;
    new_node->blue = blue;
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
    if (head != NULL)
    {
        ESP_LOGI(TAG, "Light schedule already initialized.");
        return;
    }

    initialize_storage();
    load_file("/spiffs/schema_02.csv");

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
        ESP_LOGW(TAG, "No suitable light item found for time up to %04d", hhmm);
    }
    else
    {
        ESP_LOGD(TAG, "Best light item for time %04d is %s", hhmm, best_item->time);
    }

    return best_item;
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
        vTaskDelete(NULL);
        return;
    }

    initialize_light_items();

    const int total_minutes_in_day = 24 * 60;
    long delay_ms = (long)cycle_duration_minutes * 60 * 1000 / total_minutes_in_day;
    ESP_LOGI(TAG, "Starting simulation of a 24h cycle over %d minutes. Each simulated minute will take %ld ms.",
             cycle_duration_minutes, delay_ms);

    int current_minute_of_day = 0;
    light_item_node_t *last_item = NULL;

    while (1)
    {
        int hours = current_minute_of_day / 60;
        int minutes = current_minute_of_day % 60;
        int hhmm = hours * 100 + minutes;
        time = time_to_string(hhmm);

        light_item_node_t *current_item = find_best_light_item_for_time(hhmm);

        if (current_item != NULL && current_item != last_item)
        {
            ESP_LOGI(TAG, "Simulating time: %02d:%02d -> Closest schedule is %s. R:%d, G:%d, B:%d", hours, minutes,
                     current_item->time, current_item->red, current_item->green, current_item->blue);
            led_strip_update(
                LED_STATE_SIMULATION,
                (rgb_t){.red = current_item->red, .green = current_item->green, .blue = current_item->blue});
            last_item = current_item;
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
    if (simulation_task_handle != NULL)
    {
        vTaskDelete(simulation_task_handle);
        simulation_task_handle = NULL;
    }

    simulation_config_t *config =
        (simulation_config_t *)heap_caps_malloc(sizeof(simulation_config_t), MALLOC_CAP_SPIRAM);
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for simulation config.");
        return;
    }

    config->cycle_duration_minutes = 15;

    if (xTaskCreate(simulate_cycle, "simulate_cycle", 4096, (void *)config, tskIDLE_PRIORITY + 1,
                    &simulation_task_handle) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create simulation task.");
        heap_caps_free(config);
    }
}
