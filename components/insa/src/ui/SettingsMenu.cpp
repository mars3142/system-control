#include "ui/SettingsMenu.h"

void demo(uint8_t id, ButtonType button)
{
    ///
}

SettingsMenu::SettingsMenu(menu_options_t *options) : PSMenu(options)
{
    addText(1, "OTA Einspielen", demo);
}