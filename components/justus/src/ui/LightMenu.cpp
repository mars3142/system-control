#include "ui/LightMenu.h"

void demoL(uint8_t id)
{
    //
}

LightMenu::LightMenu(menu_options_t *options) : PSMenu(options)
{
    addText("Tag/Nacht", nullptr);
    addText("LED Einstellungen", demoL);
}
