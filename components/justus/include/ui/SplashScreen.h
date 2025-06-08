#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"

class SplashScreen final : public Widget
{
  public:
    explicit SplashScreen(menu_options_t *options);
    void update(uint64_t dt) override;
    void render() override;

  private:
    menu_options_t *m_options;
};
