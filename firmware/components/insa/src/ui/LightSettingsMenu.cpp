#include "ui/LightSettingsMenu.h"

#include "common/Common.h"
#include "ui/DayColorSettingsMenu.h"
#include "ui/NightColorSettingsMenu.h"

/**
 * @namespace LightSettingsMenuItem
 * @brief Constants for light settings menu item identifiers
 */
namespace LightSettingsMenuItem
{
constexpr uint8_t RGB_SETTING_DAY = 0;
constexpr uint8_t RGB_SETTING_NIGHT = 1;
constexpr uint8_t SECTION_COUNTER = 2;
} // namespace LightSettingsMenuItem

std::string LightSettingsMenu::CreateKey(const int index)
{
    constexpr int key_length = 20;
    char key[key_length] = "";
    snprintf(key, key_length, "section_%d", index);
    return key;
}

LightSettingsMenu::LightSettingsMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    addText(LightSettingsMenuItem::RGB_SETTING_DAY, "Tag (Farbe)");
    addText(LightSettingsMenuItem::RGB_SETTING_NIGHT, "Nacht (Farbe)");

    /*
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

    setItemSize(std::stoull(getItem(LightSettingsMenuItem::SECTION_COUNTER).getValue()),
                LightSettingsMenuItem::SECTION_COUNTER);
    */
}

void LightSettingsMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    std::shared_ptr<Widget> widget;

    switch (button)
    {
    case ButtonType::SELECT:
        switch (menuItem.getId())
        {
        case LightSettingsMenuItem::RGB_SETTING_DAY:
            widget = std::make_shared<DayColorSettingsMenu>(m_options);
            break;

        case LightSettingsMenuItem::RGB_SETTING_NIGHT:
            widget = std::make_shared<NightColorSettingsMenu>(m_options);
            break;

        default:
            break;
        }

        if (m_options && m_options->pushScreen)
        {
            m_options->pushScreen(widget);
        }
        break;

    case ButtonType::RIGHT:
    case ButtonType::LEFT:
        // Handle value switching for the current menu item
        switchValue(menuItem, button);

        // Update the section list size based on the section counter value
        if (menuItem.getId() == LightSettingsMenuItem::SECTION_COUNTER)
        {
            setItemSize(std::stoull(getItem(LightSettingsMenuItem::SECTION_COUNTER).getValue()),
                        LightSettingsMenuItem::SECTION_COUNTER);
        }

        // Persist the changed section values if persistence is available
        if (m_options && m_options->persistenceManager)
        {
            const auto value = getItem(menuItem.getId()).getIndex();
            m_options->persistenceManager->SetValue(CreateKey(menuItem.getId()), value);
        }
        break;

    default:
        break;
    }
}
