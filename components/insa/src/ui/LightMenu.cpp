#include "ui/LightMenu.h"

#include "ui/LightSettingsMenu.h"

namespace LightMenuItem
{
constexpr uint8_t MODE = 1;
constexpr uint8_t TOGGLE = 2;
constexpr uint8_t LED_SETTINGS = 3;
}

LightMenu::LightMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    std::vector<std::string> values;
    values.emplace_back("Tag");
    values.emplace_back("Nacht");
    addSelection(LightMenuItem::MODE, "Modus", values, 0);

    addText(LightMenuItem::LED_SETTINGS, "LED Einstellungen");
}

void LightMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    std::shared_ptr<Widget> widget;
    switch (menuItem.getId())
    {
    case LightMenuItem::LED_SETTINGS:
        widget = std::make_shared<LightSettingsMenu>(m_options);
        break;

    default:
        break;
    }

    if (m_options && m_options->pushScreen)
    {
        m_options->pushScreen(widget);
    }
}