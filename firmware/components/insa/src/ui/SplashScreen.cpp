#include "ui/SplashScreen.h"

#include "ui/MainMenu.h"

uint64_t splashTime = 0;

SplashScreen::SplashScreen(menu_options_t *options) : Widget(options->u8g2), m_options(options)
{
}

void SplashScreen::Update(const uint64_t dt)
{
    splashTime += dt;
    if (splashTime > 100)
    {
        if (m_options && m_options->setScreen)
        {
            m_options->setScreen(std::make_shared<MainMenu>(m_options));
        }
    }
}

void SplashScreen::Render()
{
    u8g2_SetFont(u8g2, u8g2_font_DigitalDisco_tr);
    u8g2_DrawStr(u8g2, 28, u8g2->height / 2 - 10, "HO Anlage");
    u8g2_DrawStr(u8g2, 30, u8g2->height / 2 + 5, "Axel Janz");
    u8g2_SetFont(u8g2, u8g2_font_haxrcorp4089_tr);
    u8g2_DrawStr(u8g2, 35, 50, "Initialisierung...");
}
