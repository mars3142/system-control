#include "ui/SettingsMenu.h"

void demo(uint8_t id)
{
    ///
}

SettingsMenu::SettingsMenu(menu_options_t* options) : PSMenu(options)
{
    addText("OTA Einspielen", demo);
}
