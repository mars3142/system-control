#include "ui/LightSettingsMenu.h"

/**
 * @namespace LightSettingsMenuItem
 * @brief Constants for light settings menu item identifiers
 */
namespace LightSettingsMenuItem
{
constexpr uint8_t SECTION_COUNTER = 0; ///< ID for the section counter menu item
}

std::string LightSettingsMenu::CreateKey(const int index)
{
    constexpr int key_length = 20;
    char key[key_length] = "";
    snprintf(key, key_length, "section_%d", index);
    return key;
}

LightSettingsMenu::LightSettingsMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    // Create values vector for section counts (1-99)
    std::vector<std::string> values;
    for (size_t i = 1; i <= 99; i++)
    {
        values.emplace_back(std::to_string(i));
    }

    // Add section counter selection (allows choosing number of sections)
    auto value = 7;
    if (m_options && m_options->persistenceManager)
    {
        value = m_options->persistenceManager->GetValue(CreateKey(0), value);
    }
    addSelection(LightSettingsMenuItem::SECTION_COUNTER, "Sektionen", values, value);

    setItemSize(std::stoull(getItem(0).getValue()));
}

void LightSettingsMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    // Handle value switching for the current menu item
    switchValue(menuItem, button);

    // Update the section list size based on the section counter value
    if (menuItem.getId() == 0)
    {
        setItemSize(std::stoull(getItem(0).getValue()));
    }

    // Persist the changed section values if persistence is available
    if (m_options && m_options->persistenceManager)
    {
        const auto value = getItem(menuItem.getId()).getIndex();
        m_options->persistenceManager->SetValue(CreateKey(menuItem.getId()), value);
    }
}