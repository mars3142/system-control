#pragma once

#include "esp_check.h"
#include <stdint.h>

// Configuration structure for the simulation
typedef struct
{
    int cycle_duration_minutes;
} simulation_config_t;

__BEGIN_DECLS
char *get_time(void);

esp_err_t add_light_item(const char time[5], uint8_t red, uint8_t green, uint8_t blue);
void cleanup_light_items(void);
void start_simulate_day(void);
void start_simulate_night(void);
void start_simulation_task(void);
__END_DECLS
