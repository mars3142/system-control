#include "ui/MainMenu.h"

#include "common/Widget.h"
#include "ui/LightMenu.h"
#include "ui/SettingsMenu.h"

MainMenu::MainMenu(menu_options_t *options) : PSMenu(options), m_options(options)
{
    addText("Lichtsteuerung", [this](const uint8_t button) { onSelect(button); });
    addText("Einstellungen", [this](const uint8_t button) { onSelect(button); });
}

void MainMenu::onSelect(const uint8_t id) const
{
    std::shared_ptr<Widget> widget;
    switch (id)
    {
    case 0:
        widget = std::make_shared<LightMenu>(m_options);
        break;
    case 1:
        widget = std::make_shared<SettingsMenu>(m_options);
        break;
    }
    if (m_options && m_options->pushScreen)
    {
        m_options->pushScreen(widget);
    }
}
