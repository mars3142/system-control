#include "ui/MainMenu.h"

#include "common/Widget.h"
#include "ui/LightMenu.h"
#include "ui/SettingsMenu.h"

MainMenu::MainMenu(menu_options_t *options) : PSMenu(options), m_options(options)
{
    addText(1, "Lichtsteuerung", [this](const uint8_t id, const ButtonType button) {
        onButtonPressed(id, button);
    });
    addText(2, "externe Geraete", [this](const uint8_t id, const ButtonType button) {
        onButtonPressed(id, button);
    });
    addText(3, "Einstellungen", [this](const uint8_t id, const ButtonType button) {
        onButtonPressed(id, button);
    });
}

void MainMenu::onButtonPressed(const uint8_t id, const ButtonType button) const
{
    std::shared_ptr<Widget> widget;
    switch (id)
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