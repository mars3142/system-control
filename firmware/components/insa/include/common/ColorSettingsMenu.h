#pragma once

#include "common/Menu.h"

namespace ColorSettingsMenuOptions
{
constexpr auto RED = "red_";
constexpr auto GREEN = "green_";
constexpr auto BLUE = "blue_";
} // namespace ColorSettingsMenuOptions

class ColorSettingsMenu : public Menu
{
  public:
    /**
     * @brief Constructs a ColorSettingsMenu with the specified options
     * @param options Pointer to menu configuration options structure
     * @details Initializes the menu with color settings options
     */
    explicit ColorSettingsMenu(menu_options_t *options, std::string prefix);

    void onButtonPressed(const MenuItem &menuItem, const ButtonType button) override;

    void onExit() override;

  private:
    std::string m_suffix;
    menu_options_t *m_options;
};
