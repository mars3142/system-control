#include "ui/LightMenu.h"

#include "ui/LightSettingsMenu.h"

/**
 * @namespace LightMenuItem
 * @brief Constants for light menu item identifiers
 */
namespace LightMenuItem
{
constexpr uint8_t ACTIVATE = 0;      ///< ID for the light activation toggle
constexpr uint8_t MODE = 1;          ///< ID for the light mode selection
constexpr uint8_t LED_SETTINGS = 2;  ///< ID for the LED settings menu item
} // namespace LightMenuItem

LightMenu::LightMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    // Add toggle for enabling/disabling the light system
    addToggle(LightMenuItem::ACTIVATE, "Einschalten", true);

    // Create mode selection options (Day/Night modes)
    std::vector<std::string> values;
    values.emplace_back("Tag");    // Day mode
    values.emplace_back("Nacht");  // Night mode
    addSelection(LightMenuItem::MODE, "Modus", values, 0);

    // Add menu item for accessing LED settings submenu
    addText(LightMenuItem::LED_SETTINGS, "Einstellungen");
}

void LightMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    MenuItem item = menuItem;
    std::shared_ptr<Widget> widget;
    
    // Handle different menu items based on their ID
    switch (menuItem.getId())
    {
    case LightMenuItem::ACTIVATE: {
        // Toggle the light activation state when SELECT is pressed
        if (button == ButtonType::SELECT)
        {
            toggle(menuItem);
        }
        if (m_options && m_options->persistence && m_options->persistence->save)
        {
            const auto value = getItem(item.getId()).getValue();
            m_options->persistence->save(VALUE_TYPE_STRING, "light_activated", value.c_str());
        }
        break;
    }

    case LightMenuItem::MODE: {
        // Switch between day/night modes using left/right buttons
        item = switchValue(menuItem, button);
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            if (m_options && m_options->persistence && m_options->persistence->save)
            {
                const auto value = getItem(item.getId()).getIndex();
                m_options->persistence->save(VALUE_TYPE_INT32, "light_mode", &value);
            }
        }
        break;
    }

    case LightMenuItem::LED_SETTINGS: {
        // Open the LED settings submenu when SELECT is pressed
        if (button == ButtonType::SELECT)
        {
            widget = std::make_shared<LightSettingsMenu>(m_options);
        }
        break;
    }

    default:
        // Handle unknown menu items (no action required)
        break;
    }

    // Push the new widget to the screen stack if one was created
    if (m_options && m_options->pushScreen)
    {
        m_options->pushScreen(widget);
    }
}