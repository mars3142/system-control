#include "hal/u8g2_hal_sdl.h"

#include <SDL3/SDL.h>
#include <cstdint>

uint8_t u8x8_byte_sdl_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
    case U8X8_MSG_BYTE_INIT:
    case U8X8_MSG_BYTE_SET_DC:
    case U8X8_MSG_BYTE_START_TRANSFER:
    case U8X8_MSG_BYTE_END_TRANSFER:
        break;

    default:
        return 0;
    }
    return 1;
}

uint8_t u8x8_gpio_and_delay_sdl(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_MILLI:
        SDL_Delay(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        SDL_Delay((arg_int + 99) / 100);
        break;

    case U8X8_MSG_DELAY_100NANO:
    case U8X8_MSG_DELAY_NANO:
        SDL_Delay(1);
        break;

    case U8X8_MSG_GPIO_AND_DELAY_INIT:
    case U8X8_MSG_GPIO_RESET:
    case U8X8_MSG_GPIO_CS:
    case U8X8_MSG_GPIO_DC:
        break;

    default:
        break;
    }
    return 1;
}
