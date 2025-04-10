#pragma once

#include "common/PSMenu.h"

class MainMenu final : public PSMenu {
public:
    explicit MainMenu(menu_options_t* options);

private:
    void onSelect(uint8_t id) const;

    menu_options_t* m_options;
};
