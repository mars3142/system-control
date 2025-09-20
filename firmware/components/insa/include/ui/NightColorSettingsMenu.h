
#pragma once

#include "common/ColorSettingsMenu.h"

class NightColorSettingsMenu final : public ColorSettingsMenu
{
  public:
    /**
     * @brief Constructs a NightColorSettingsMenu with the specified options
     * @param options Pointer to menu configuration options structure
     * @details Initializes the menu with night color settings options
     */
    explicit NightColorSettingsMenu(menu_options_t *options);
};
