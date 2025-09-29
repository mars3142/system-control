/**
 * @file ClockScreenSaver.h
 * @brief Animated clock screensaver implementation with bouncing time display
 * @details This header defines the ClockScreenSaver class which provides an animated
 *          clock screensaver that shows the system time bouncing around the screen.
 *          The screensaver prevents screen burn-in while displaying useful information.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"

/**
 * @class ClockScreenSaver
 * @brief Animated clock screensaver widget for system idle periods
 */
class ClockScreenSaver final : public Widget
{
  public:
    explicit ClockScreenSaver(menu_options_t *options);
    void Update(uint64_t dt) override;
    void Render() override;
    void OnButtonClicked(ButtonType button) override;

  private:
    static constexpr int MOVE_INTERVAL = 50;                       // milliseconds between movements
    static constexpr int X_VELOCITY = 1;                           // horizontal movement speed
    static constexpr int Y_VELOCITY = 1;                           // vertical movement speed
    static constexpr int TEXT_PADDING = 0;                         // padding around text for bounds checking
    static constexpr const uint8_t *FONT = u8g2_font_profont15_tf; // font for time display

    menu_options_t *m_options;
    uint64_t m_moveTimer;

    // Position and movement
    int m_posX;
    int m_posY;
    int m_velocityX;
    int m_velocityY;

    // Text dimensions (calculated once)
    int m_textWidth;
    int m_textHeight;

    /**
     * @brief Initialize position and movement
     */
    void initPosition();

    /**
     * @brief Update the text dimensions based on current time
     */
    void updateTextDimensions();

    /**
     * @brief Get the current time as a formatted string
     * @param buffer Buffer to store the formatted time
     * @param bufferSize Size of the buffer
     */
    void getCurrentTimeString(char *buffer, size_t bufferSize) const;

    /**
     * @brief Check and handle collision with screen boundaries
     */
    void checkBoundaryCollision();
};
