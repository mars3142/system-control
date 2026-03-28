#include "hermes/screensaver/clock_screensaver.h"

#include <ctime>

ClockScreensaver::ClockScreensaver(u8g2_t *u8g2) : m_u8g2(u8g2)
{
}

void ClockScreensaver::get_time_string(char *buffer, size_t bufferSize)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, bufferSize, "%H:%M:%S", timeinfo);
}

void ClockScreensaver::init()
{
    u8g2_SetFont(m_u8g2, FONT);
    char timeBuf[32];
    get_time_string(timeBuf, sizeof(timeBuf));
    int tw = u8g2_GetStrWidth(m_u8g2, timeBuf);
    int th = u8g2_GetAscent(m_u8g2) - u8g2_GetDescent(m_u8g2);

    m_posX = (m_u8g2->width - tw) / 2;
    m_posY = (m_u8g2->height - th) / 2;
    m_velX = VELOCITY;
    m_velY = VELOCITY;
    m_moveTimer = 0;
}

void ClockScreensaver::draw(uint64_t dt)
{
    m_moveTimer += dt;

    if (m_moveTimer >= MOVE_INTERVAL_MS)
    {
        m_moveTimer = 0;

        char timeBuf[32];
        get_time_string(timeBuf, sizeof(timeBuf));
        u8g2_SetFont(m_u8g2, FONT);
        int tw = u8g2_GetStrWidth(m_u8g2, timeBuf);
        int th = u8g2_GetAscent(m_u8g2) - u8g2_GetDescent(m_u8g2);

        m_posX += m_velX;
        m_posY += m_velY;

        if (m_posX <= 0)
        {
            m_posX = 0;
            m_velX = VELOCITY;
        }
        else if (m_posX + tw >= m_u8g2->width)
        {
            m_posX = m_u8g2->width - tw;
            m_velX = -VELOCITY;
        }

        if (m_posY <= th)
        {
            m_posY = th;
            m_velY = VELOCITY;
        }
        else if (m_posY >= m_u8g2->height)
        {
            m_posY = m_u8g2->height;
            m_velY = -VELOCITY;
        }
    }

    char timeBuf[32];
    get_time_string(timeBuf, sizeof(timeBuf));
    u8g2_SetFont(m_u8g2, FONT);
    u8g2_DrawStr(m_u8g2, m_posX, m_posY, timeBuf);
}
