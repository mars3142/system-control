#pragma once

#include "common/PSMenu.h"

class MainMenu final : public PSMenu
{
public:
    explicit MainMenu(menu_options_t *options);

private:
    void onButtonPressed(uint8_t id, ButtonType button) const;

    menu_options_t *m_options;
};