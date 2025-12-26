#pragma once

#include "common/Menu.h"

class ExternalDevices final : public Menu
{
  public:
    explicit ExternalDevices(menu_options_t *options);

    const char *getName() const override;

  private:
    void onButtonPressed(const MenuItem &menuItem, ButtonType button) override;
    menu_options_t *m_options;
};
