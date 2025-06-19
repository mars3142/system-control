#include "ui/SplashScreen.h"

#include "ui/MainMenu.h"

#ifndef ESP32
#include <chrono>
#include <thread>
#endif

uint64_t counter = 0;

SplashScreen::SplashScreen(menu_options_t *options) : Widget(options->u8g2), m_options(options)
{
}

void SplashScreen::update(const uint64_t dt)
{
    counter += dt;
#ifndef ESP32
    if (counter > 3000)
#else
    if (counter > 10)
#endif
    {
        counter = 0;
        if (m_options && m_options->setScreen)
        {
            m_options->setScreen(std::make_shared<MainMenu>(m_options));
        }
    }
#ifndef ESP32
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
}

void SplashScreen::render()
{
    u8g2_SetFont(u8g2, u8g2_font_DigitalDisco_tr);
    u8g2_DrawStr(u8g2, 28, u8g2->height / 2 - 10, "HO Anlage");
    u8g2_DrawStr(u8g2, 30, u8g2->height / 2 + 5, "Axel Janz");
    u8g2_SetFont(u8g2, u8g2_font_haxrcorp4089_tr);
    u8g2_DrawStr(u8g2, 35, 50, "Initialisierung...");
}