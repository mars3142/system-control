#include "ui/LightMenu.h"

LightMenu::LightMenu(menu_options_t *options) : PSMenu(options), m_options(options)
{
    std::vector<std::string> values;
    values.emplace_back("Tag");
    values.emplace_back("Nacht");
    addSelection(1, "Modus", values.front(), values, [this](const uint8_t id, const ButtonType button) {
        onButtonPressed(id, button);
    });

    addText(2, "LED Einstellungen", [this](const uint8_t id, const ButtonType button) {
        onButtonPressed(id, button);
    });

    addToggle(3, "Toggle", false, nullptr);
}

void LightMenu::onButtonPressed(const uint8_t id, const ButtonType button) const
{
    std::shared_ptr<Widget> widget;
    switch (id)
    {
    case 2:
        widget = std::make_shared<LightMenu>(m_options);
        break;

    default:
        break;
    }

    if (m_options && m_options->pushScreen)
    {
        m_options->pushScreen(widget);
    }
}