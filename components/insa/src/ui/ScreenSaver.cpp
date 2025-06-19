#include "ui/ScreenSaver.h"
#include <cmath>

ScreenSaver::ScreenSaver(menu_options_t *options) : Widget(options->u8g2), m_options(options), m_animationCounter(0)
{
    initStars();
}

void ScreenSaver::initStars()
{
    m_stars.resize(NUM_STARS);

    for (auto &star : m_stars)
    {
        resetStar(star);
        star.z = Z_NEAR + (static_cast<float>(rand()) / RAND_MAX) * (Z_FAR - Z_NEAR);
    }
}

void ScreenSaver::resetStar(Star &star)
{
    star.x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
    star.y = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
    star.z = Z_FAR;
    star.speed = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.5f;
}

void ScreenSaver::update(const uint64_t dt)
{
    m_animationCounter += dt;

    if (m_animationCounter > 8)
    {
        m_animationCounter = 0;

        for (auto &star : m_stars)
        {
            star.z -= star.speed * SPEED_MULTIPLIER;

            if (star.z < Z_NEAR)
            {
                resetStar(star);
            }
        }
    }
}

void ScreenSaver::render()
{
    // Verwende Page-Buffer Mode statt Full-Buffer für bessere Performance
    // Schwarzer Hintergrund
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);
    u8g2_SetDrawColor(u8g2, 1);

    const int centerX = u8g2->width / 2;
    const int centerY = u8g2->height / 2;

    // Zeichne nur sichtbare Sterne (Clipping)
    for (const auto &star : m_stars)
    {
        // 3D zu 2D Projektion
        int screenX = centerX + static_cast<int>((star.x / star.z) * centerX);
        int screenY = centerY + static_cast<int>((star.y / star.z) * centerY);

        // Frühe Prüfung für Performance
        if (screenX < -5 || screenX >= u8g2->width + 5 || screenY < -5 || screenY >= u8g2->height + 5)
        {
            continue;
        }

        // Vereinfachte Sterndarstellung für bessere Performance
        int size = static_cast<int>((1.0f - (star.z - Z_NEAR) / (Z_FAR - Z_NEAR)) * 2.0f);

        if (size <= 0)
        {
            u8g2_DrawPixel(u8g2, screenX, screenY);
        }
        else
        {
            // Verwende u8g2_DrawCircle für größere Sterne (schneller)
            u8g2_DrawCircle(u8g2, screenX, screenY, size, U8G2_DRAW_ALL);
        }
    }
}

void ScreenSaver::onButtonClicked(ButtonType button)
{
    if (m_options && m_options->popScreen)
    {
        m_options->popScreen();
    }
}
