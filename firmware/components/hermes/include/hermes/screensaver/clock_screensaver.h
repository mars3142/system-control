#pragma once

#include "hermes/screensaver/screensaver.h"

#include <u8g2.h>
#include <cstdint>

class ClockScreensaver : public Screensaver
{
  public:
    explicit ClockScreensaver(u8g2_t *u8g2);

    void init() override;
    void draw(uint64_t dt) override;

  private:
    u8g2_t *m_u8g2;

    int m_posX = 0;
    int m_posY = 0;
    int m_velX = 1;
    int m_velY = 1;
    uint64_t m_moveTimer = 0;

    static constexpr int MOVE_INTERVAL_MS = 50;
    static constexpr int VELOCITY = 1;
    static constexpr const uint8_t *FONT = u8g2_font_profont15_tf;

    static void get_time_string(char *buffer, size_t bufferSize);
};
