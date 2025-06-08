#pragma once

#include "MenuOptions.h"
#include "Widget.h"

class ScrollBar final : public Widget
{
  public:
    ScrollBar(const menu_options_t *options, size_t x, size_t y, size_t width, size_t height);
    void render() override;
    void refresh(size_t value, size_t max, size_t min = 0);

  private:
    size_t m_x;
    size_t m_y;
    size_t m_width;
    size_t m_height;
    size_t m_value;
    size_t m_max;
    size_t m_min;

    size_t m_thumbHeight;
    size_t m_thumbY;
};
