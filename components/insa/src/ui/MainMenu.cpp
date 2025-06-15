#include "ui/MainMenu.h"

#include "common/Widget.h"
#include "ui/LightMenu.h"
#include "ui/SettingsMenu.h"

namespace MainMenuItem
{
constexpr uint8_t LIGHT = 0;
constexpr uint8_t EXTERNAL_DEVICES = 1;
constexpr uint8_t SETTINGS = 2;
} // namespace MainMenuItem

MainMenu::MainMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    addText(MainMenuItem::LIGHT, "Lichtsteuerung");
    addText(MainMenuItem::EXTERNAL_DEVICES, "Externe Geraete");
    addText(MainMenuItem::SETTINGS, "Einstellungen");
}

void MainMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    if (button == ButtonType::SELECT)
    {
        std::shared_ptr<Widget> widget;
        switch (menuItem.getId())
        {
        case MainMenuItem::LIGHT:
            widget = std::make_shared<LightMenu>(m_options);
            break;

        case MainMenuItem::SETTINGS:
            widget = std::make_shared<SettingsMenu>(m_options);
            break;
        default:
            break;
        }

        if (m_options && m_options->pushScreen)
        {
            m_options->pushScreen(widget);
        }
    }
}