#pragma once

#include "u8g2.h"

#define U8G2_SCREEN_WIDTH (128)
#define U8G2_SCREEN_HEIGHT (64)
#define U8G2_SCREEN_FACTOR (3)
#define U8G2_SCREEN_PADDING (25)

uint8_t u8x8_byte_sdl_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay_sdl(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
