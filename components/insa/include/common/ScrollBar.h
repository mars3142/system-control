#pragma once

#include "MenuOptions.h"
#include "Widget.h"

/**
 * ScrollBar class that represents a vertical scrollbar widget
 * Inherits from Widget base class and provides scrolling functionality
 */
class ScrollBar final : public Widget
{
public:
    /**
     * Constructor for ScrollBar
     * @param options Pointer to menu options configuration
     * @param x X coordinate position of the scrollbar
     * @param y Y coordinate position of the scrollbar
     * @param width Width of the scrollbar
     * @param height Height of the scrollbar
     */
    ScrollBar(const menu_options_t *options, size_t x, size_t y, size_t width, size_t height);

    /**
     * Renders the scrollbar to the screen
     * Overrides the base Widget render method
     */
    void render() override;

    /**
     * Updates the scrollbar state with new values
     * @param value Current scroll position value
     * @param max Maximum scroll value
     * @param min Minimum scroll value (default: 0)
     */
    void refresh(size_t value, size_t max, size_t min = 0);

private:
    // Position and dimensions
    size_t m_x;      // X coordinate of the scrollbar
    size_t m_y;      // Y coordinate of the scrollbar
    size_t m_width;  // Width of the scrollbar
    size_t m_height; // Height of the scrollbar

    // Scroll state values
    size_t m_value;  // Current scroll position
    size_t m_max;    // Maximum scroll value
    size_t m_min;    // Minimum scroll value

    // Calculated thumb properties
    size_t m_thumbHeight; // Height of the scroll thumb
    size_t m_thumbY;      // Y position of the scroll thumb
};