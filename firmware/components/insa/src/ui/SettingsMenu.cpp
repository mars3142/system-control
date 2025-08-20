#include "ui/SettingsMenu.h"

namespace SettingsMenuItem
{
constexpr uint8_t OTA_UPLOAD = 0;
}

SettingsMenu::SettingsMenu(menu_options_t *options) : Menu(options)
{
    addText(SettingsMenuItem::OTA_UPLOAD, "OTA Einspielen");
}