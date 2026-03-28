#pragma once

#include <u8g2.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initializes the menu renderer with the display context
     * @param u8g2 Display context
     * @param inactivity_timeout_ms Timeout in ms before screensaver activates (0 = disabled)
     */
    void hermes_init(u8g2_t *u8g2, uint32_t inactivity_timeout_ms);

    /**
     * @brief Renders the current state to the display (splash, menu, or screensaver)
     * @param dt Delta time in milliseconds since the last call
     */
    void hermes_draw(uint64_t dt);

    /**
     * @brief Resets the inactivity timer (call on button input)
     */
    void hermes_reset_inactivity(void);

#ifdef __cplusplus
}
#endif
