#include "ui/LightMenu.h"

#include "led_strip_ws2812.h"
#include "simulator.h"

/**
 * @namespace LightMenuItem
 * @brief Constants for light menu item identifiers
 */
namespace LightMenuItem
{
constexpr auto ACTIVATE = 0; ///< ID for the light activation toggle
constexpr auto MODE = 1;     ///< ID for the light mode selection
} // namespace LightMenuItem

namespace LightMenuOptions
{
constexpr auto LIGHT_ACTIVE = "light_active";
constexpr auto LIGHT_MODE = "light_mode";
} // namespace LightMenuOptions

LightMenu::LightMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    // Add toggle for enabling/disabling the light system
    bool active = false;
    if (m_options && m_options->persistenceManager)
    {
        active = m_options->persistenceManager->GetValue(LightMenuOptions::LIGHT_ACTIVE, active);
    }
    addToggle(LightMenuItem::ACTIVATE, "Einschalten", active);

    // Create mode selection options (Simulation/Day/Night modes)
    std::vector<std::string> items;
    items.emplace_back("Simulation"); // Simulation mode
    items.emplace_back("Tag");        // Day mode
    items.emplace_back("Nacht");      // Night mode
    int mode_value = 0;
    if (m_options && m_options->persistenceManager)
    {
        mode_value = m_options->persistenceManager->GetValue(LightMenuOptions::LIGHT_MODE, mode_value);
    }
    addSelection(LightMenuItem::MODE, "Modus", items, mode_value);
}

void LightMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    std::shared_ptr<Widget> widget;

    // Handle different menu items based on their ID
    switch (menuItem.getId())
    {
    case LightMenuItem::ACTIVATE: {
        // Toggle the light activation state when SELECT is pressed
        if (button == ButtonType::SELECT)
        {
            toggle(menuItem);
            const auto value = getItem(menuItem.getId()).getValue() == "1";
            if (m_options && m_options->persistenceManager)
            {
                m_options->persistenceManager->SetValue(LightMenuOptions::LIGHT_ACTIVE, value);
            }

            start_simulation();
        }
        break;
    }

    case LightMenuItem::MODE: {
        // Switch between day/night modes using left/right buttons
        const auto item = switchValue(menuItem, button);
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto value = getItem(item.getId()).getIndex();
            if (m_options && m_options->persistenceManager)
            {
                m_options->persistenceManager->SetValue(LightMenuOptions::LIGHT_MODE, value);
                m_options->persistenceManager->Save();
            }

            start_simulation();
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
