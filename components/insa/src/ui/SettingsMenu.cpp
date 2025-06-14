#include "ui/SettingsMenu.h"

SettingsMenu::SettingsMenu(menu_options_t *options) : Menu(options)
{
    addText(1, "OTA Einspielen");
}