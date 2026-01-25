#include "ui/LightMenu.h"
#include "led_strip_ws2812.h"
#include "message_manager.h"
#include "simulator.h"
#include <cstring>

/**
 * @namespace LightMenuItem
 * @brief Constants for light menu item identifiers
 */
namespace LightMenuItem
{
constexpr auto ACTIVATE = 0; ///< ID for the light activation toggle
constexpr auto MODE = 1;     ///< ID for the light mode selection
constexpr auto VARIANT = 2;  ///< ID for the light variant selection
} // namespace LightMenuItem

namespace LightMenuOptions
{
constexpr auto LIGHT_ACTIVE = "light_active";
constexpr auto LIGHT_MODE = "light_mode";
constexpr auto LIGHT_VARIANT = "light_variant";
} // namespace LightMenuOptions

LightMenu::LightMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    // Add toggle for enabling/disabling the light system
    bool active = false;
    if (m_options && m_options->persistenceManager)
    {
        active = persistence_manager_get_bool(m_options->persistenceManager, LightMenuOptions::LIGHT_ACTIVE, active);
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
        mode_value =
            persistence_manager_get_int(m_options->persistenceManager, LightMenuOptions::LIGHT_MODE, mode_value);
    }
    addSelection(LightMenuItem::MODE, "Modus", items, mode_value);

    std::vector<std::string> variants;
    variants.emplace_back("1");
    variants.emplace_back("2");
    variants.emplace_back("3");
    int variant_value = 3;
    if (m_options && m_options->persistenceManager)
    {
        variant_value =
            persistence_manager_get_int(m_options->persistenceManager, LightMenuOptions::LIGHT_VARIANT, variant_value) -
            1;
    }
    addSelection(LightMenuItem::VARIANT, "Variante", variants, variant_value);
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
            // Post change via message_manager
            message_t msg = {};
            msg.type = MESSAGE_TYPE_SETTINGS;
            msg.data.settings.type = SETTINGS_TYPE_BOOL;
            strncpy(msg.data.settings.key, LightMenuOptions::LIGHT_ACTIVE, sizeof(msg.data.settings.key) - 1);
            msg.data.settings.value.bool_value = value;
            message_manager_post(&msg, pdMS_TO_TICKS(100));
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
                // Post change via message_manager
                message_t msg = {};
                msg.type = MESSAGE_TYPE_SETTINGS;
                msg.data.settings.type = SETTINGS_TYPE_INT;
                strncpy(msg.data.settings.key, LightMenuOptions::LIGHT_MODE, sizeof(msg.data.settings.key) - 1);
                msg.data.settings.value.int_value = value;
                message_manager_post(&msg, pdMS_TO_TICKS(100));
            }
        }
        break;
    }

    case LightMenuItem::VARIANT: {
        // Change light variant using left/right buttons
        const auto item = switchValue(menuItem, button);
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto value = getItem(item.getId()).getIndex() + 1;
            if (m_options && m_options->persistenceManager)
            {
                // Post change via message_manager
                message_t msg = {};
                msg.type = MESSAGE_TYPE_SETTINGS;
                msg.data.settings.type = SETTINGS_TYPE_INT;
                strncpy(msg.data.settings.key, LightMenuOptions::LIGHT_VARIANT, sizeof(msg.data.settings.key) - 1);
                msg.data.settings.value.int_value = value;
                message_manager_post(&msg, pdMS_TO_TICKS(100));
            }
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

void LightMenu::onMessageReceived(const message_t *msg)
{
    // Here you can react to messages, e.g. set toggle status
    // Example: If light_active was changed, synchronize toggle
    if (msg && msg->type == MESSAGE_TYPE_SETTINGS)
    {
        if (std::strcmp(msg->data.settings.key, LightMenuOptions::LIGHT_ACTIVE) == 0)
        {
            setToggle(getItem(LightMenuItem::ACTIVATE), msg->data.settings.value.bool_value);
        }
        if (std::strcmp(msg->data.settings.key, LightMenuOptions::LIGHT_MODE) == 0)
        {
            setSelectionIndex(getItem(LightMenuItem::MODE), msg->data.settings.value.int_value);
        }
        if (std::strcmp(msg->data.settings.key, LightMenuOptions::LIGHT_VARIANT) == 0)
        {
            setSelectionIndex(getItem(LightMenuItem::VARIANT), msg->data.settings.value.int_value - 1);
        }
    }
}

IMPLEMENT_GET_NAME(LightMenu)
