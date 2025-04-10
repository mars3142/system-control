#include "common/ScrollBar.h"

ScrollBar::ScrollBar(
    const menu_options_t* options,
    const size_t x,
    const size_t y,
    const size_t width,
    const size_t height)
    : Widget(options->u8g2)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_value(0)
    , m_max(0)
    , m_min(0)
    , m_thumbHeight(0)
    , m_thumbY(0) {
}

void ScrollBar::render() {
    if(1 == m_max) return;

    // draw dotted line
    for(size_t y = m_y; y < m_y + m_height; y++) {
        if(y % 2 == 0) {
            u8g2_DrawPixel(u8g2, m_x, y);
        }
    }

    // draw thumb
    u8g2_DrawBox(u8g2, u8g2->width - 4, m_thumbY, 3, m_thumbHeight);
}

void ScrollBar::refresh(const size_t value, const size_t max, const size_t min) {
    m_value = value;
    m_max = max;
    m_min = min;

    m_thumbHeight = m_height / m_max;
    m_thumbY = m_y + (m_value - m_min) * m_thumbHeight;
}
