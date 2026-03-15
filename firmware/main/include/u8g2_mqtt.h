#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern QueueHandle_t display_mqtt_queue;

void u8g2_mqtt_task(void *pvParameters);
