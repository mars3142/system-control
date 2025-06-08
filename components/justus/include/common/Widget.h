#pragma once

#include <cstdint>

#include "u8g2.h"

class Widget
{
  public:
    explicit Widget(u8g2_t *u8g2);

    virtual ~Widget() = default;

    virtual void update(uint64_t dt);

    virtual void render();

    virtual void onButtonClicked(uint8_t button);

  protected:
    u8g2_t *u8g2;
};
