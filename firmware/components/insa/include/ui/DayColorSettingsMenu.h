#pragma once

#include "common/ColorSettingsMenu.h"

class DayColorSettingsMenu final : public ColorSettingsMenu
{
  public:
    /**
     * @brief Constructs a DayColorSettingsMenu with the specified options
     * @param options Pointer to menu configuration options structure
     * @details Initializes the menu with day color settings options
     */
    explicit DayColorSettingsMenu(menu_options_t *options);
};
