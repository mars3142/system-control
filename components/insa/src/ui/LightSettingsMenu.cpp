#include "ui/LightSettingsMenu.h"

namespace LightSettingsMenuItem
{
constexpr uint8_t SECTION_COUNTER = 0;
}

LightSettingsMenu::LightSettingsMenu(menu_options_t *options) : Menu(options), m_options(options)
{
    std::vector<std::string> values;
    for (size_t i = 1; i <= 99; i++)
    {
        values.emplace_back(std::to_string(i));
    }
    addSelection(LightSettingsMenuItem::SECTION_COUNTER, "Sektionen", values, 0);

    addSelection(1, "Sektion 1", values, 0);
}

void LightSettingsMenu::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    /// change visible counter
    switch (button)
    {
    case ButtonType::LEFT:
        if (menuItem.getIndex() > 0)
        {
            const auto item = menuItem.copyWith(menuItem.getIndex() - 1);
            replaceItem(menuItem.getId(), item);
        }
        else
        {
            const auto item = menuItem.copyWith(menuItem.getItemCount() - 1);
            replaceItem(menuItem.getId(), item);
        }
        break;

    case ButtonType::RIGHT:
        if (menuItem.getIndex() < menuItem.getItemCount() - 1)
        {
            const auto item = menuItem.copyWith(menuItem.getIndex() + 1);
            replaceItem(menuItem.getId(), item);
        }
        else
        {
            const auto item = menuItem.copyWith(0);
            replaceItem(menuItem.getId(), item);
        }
        break;

    default:
        break;
    }

    /// change section list
    setItemSize(std::stoull(getItem(0).getValue()));

    /// persist section values
    if (m_options && m_options->persistence && m_options->persistence->save)
    {
        const auto key = "section_" + std::to_string(menuItem.getId());
        const auto &value = getItem(menuItem.getId()).getValue();
        m_options->persistence->save(key.c_str(), value.c_str());
    }
}