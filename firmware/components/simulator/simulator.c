#include "simulator.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_status.h"
#include "storage.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "simulator";

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

void simulate(void *args)
{
    ESP_LOGI(TAG, "Simulation task started with args: %p", args);

    initialize_storage();
    load_file("/spiffs/schema_02.csv");

    if (head == NULL)
    {
        ESP_LOGW(TAG, "Light schedule is empty. Simulation will not run.");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Starting simulation loop.");
    light_item_node_t *current_item = head;

    while (1)
    {
        if (current_item == NULL)
        {
            current_item = head;
            ESP_LOGI(TAG, "Reached end of schedule, restarting from head.");
        }

        ESP_LOGI(TAG, "Simulating time: %s -> R:%d, G:%d, B:%d", current_item->time, current_item->red,
                 current_item->green, current_item->blue);
        led_behavior_t led1_behavior = {
            .mode = LED_MODE_SOLID,
            .color = {.r = current_item->red, .g = current_item->green, .b = current_item->blue},
            .on_time_ms = 0,
            .off_time_ms = 0};
        led_status_set_behavior(1, led1_behavior);

        current_item = current_item->next;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    cleanup_light_items();
}
