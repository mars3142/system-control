#include "common/ColorSettingsMenu.h"

#include "led_manager.h"

namespace ColorSettingsMenuItem
{
constexpr auto RED = 0;
constexpr auto GREEN = 1;
constexpr auto BLUE = 2;
} // namespace ColorSettingsMenuItem

ColorSettingsMenu::ColorSettingsMenu(menu_options_t *options, std::string suffix)
    : Menu(options), m_suffix(std::move(suffix)), m_options(options)
{

    std::vector<std::string> values;
    for (size_t i = 0; i <= 254; i++)
    {
        values.emplace_back(std::to_string(i));
    }

    int red_value = 0;
    if (m_options && m_options->persistenceManager)
    {
        std::string key = ColorSettingsMenuOptions::RED + m_suffix;
        red_value = m_options->persistenceManager->GetValue(key, red_value);
    }
    addSelection(ColorSettingsMenuItem::RED, "Rot", values, red_value);

    int green_value = 0;
    if (m_options && m_options->persistenceManager)
    {
        std::string key = ColorSettingsMenuOptions::GREEN + m_suffix;
        green_value = m_options->persistenceManager->GetValue(key, green_value);
    }
    addSelection(ColorSettingsMenuItem::GREEN, "Gruen", values, green_value);

    int blue_value = 0;
    if (m_options && m_options->persistenceManager)
    {
        std::string key = ColorSettingsMenuOptions::BLUE + m_suffix;
        blue_value = m_options->persistenceManager->GetValue(key, blue_value);
    }
    addSelection(ColorSettingsMenuItem::BLUE, "Blau", values, blue_value);
}

void ColorSettingsMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    switch (menuItem.getId())
    {
    case ColorSettingsMenuItem::RED:
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto item = switchValue(menuItem, button);
            const auto value = getItem(item.getId()).getIndex();
            if (m_options && m_options->persistenceManager)
            {
                std::string key = ColorSettingsMenuOptions::RED + m_suffix;
                m_options->persistenceManager->SetValue(key, value);
            }
        }
        break;

    case ColorSettingsMenuItem::GREEN:
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto item = switchValue(menuItem, button);
            const auto value = getItem(item.getId()).getIndex();
            if (m_options && m_options->persistenceManager)
            {
                std::string key = ColorSettingsMenuOptions::GREEN + m_suffix;
                m_options->persistenceManager->SetValue(key, value);
            }
        }
        break;

    case ColorSettingsMenuItem::BLUE:
        if (button == ButtonType::LEFT || button == ButtonType::RIGHT)
        {
            const auto item = switchValue(menuItem, button);
            const auto value = getItem(item.getId()).getIndex();
            if (m_options && m_options->persistenceManager)
            {
                std::string key = ColorSettingsMenuOptions::BLUE + m_suffix;
                m_options->persistenceManager->SetValue(key, value);
            }
        }
        break;
    }
}

void ColorSettingsMenu::onExit()
{
    if (m_options && m_options->persistenceManager)
    {
        m_options->persistenceManager->Save();

        led_event_data_t payload = {.value = 42};
        send_event(EVENT_LED_REFRESH, &payload);
    }
}
