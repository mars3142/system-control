#include "ui/LightMenu.h"

#include "led_manager.h"
#include "ui/LightSettingsMenu.h"

/**
 * @namespace LightMenuItem
 * @brief Constants for light menu item identifiers
 */
namespace LightMenuItem
{
constexpr auto ACTIVATE = 0;     ///< ID for the light activation toggle
constexpr auto MODE = 1;         ///< ID for the light mode selection
constexpr auto LED_SETTINGS = 2; ///< ID for the LED settings menu item
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

    // Create mode selection options (Day/Night modes)
    std::vector<std::string> values;
    values.emplace_back("Tag");   // Day mode
    values.emplace_back("Nacht"); // Night mode
    int mode_value = 0;
    if (m_options && m_options->persistenceManager)
    {
        mode_value = m_options->persistenceManager->GetValue(LightMenuOptions::LIGHT_MODE, mode_value);
    }
    addSelection(LightMenuItem::MODE, "Modus", values, mode_value);

    // Add menu item for accessing LED settings submenu
    addText(LightMenuItem::LED_SETTINGS, "Einstellungen");
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
            if (value)
            {
                led_event_data_t payload = {.value = 42};
                send_event(EVENT_LED_ON, &payload);
            }
            else
            {
                led_event_data_t payload = {.value = 0};
                send_event(EVENT_LED_OFF, &payload);
            }
            if (m_options && m_options->persistenceManager)
            {
                m_options->persistenceManager->SetValue(LightMenuOptions::LIGHT_ACTIVE, value);
                m_options->persistenceManager->Save();
            }
        }
        break;
    }

    case LightMenuItem::MODE: {
        // Switch between day/night modes using left/right buttons
        const auto item = switchValue(menuItem, button);
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto value = getItem(item.getId()).getIndex();
            led_event_data_t payload = {.value = value};
            send_event(EVENT_LED_DAY + value, &payload);

            if (m_options && m_options->persistenceManager)
            {
                m_options->persistenceManager->SetValue(LightMenuOptions::LIGHT_MODE, value);
                m_options->persistenceManager->Save();
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