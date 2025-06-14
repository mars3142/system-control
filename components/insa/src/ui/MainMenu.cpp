#include "ui/MainMenu.h"

#include "common/Widget.h"
#include "ui/LightMenu.h"
#include "ui/SettingsMenu.h"

MainMenu::MainMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    addText(1, "Lichtsteuerung");
    addText(2, "externe Geraete");
    addText(3, "Einstellungen");
}

void MainMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    std::shared_ptr<Widget> widget;
    switch (menuItem.getId())
    {
    case 1:
        widget = std::make_shared<LightMenu>(m_options);
        break;

    case 3:
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