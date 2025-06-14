/**
 * @file ScrollBar.h
 * @brief Vertical scrollbar widget for indicating scroll position in long content
 * @details This header defines the ScrollBar class which provides a visual scrollbar
 *          widget for indicating the current position within scrollable content.
 *          The scrollbar displays a thumb that moves proportionally to represent
 *          the current scroll position and visible area relative to the total content.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "MenuOptions.h"
#include "Widget.h"

/**
 * @class ScrollBar
 * @brief A vertical scrollbar widget that represents scroll position and range
 * @details This final class inherits from Widget and provides visual feedback for
 *          scrollable content. It displays a vertical track with a movable thumb
 *          that indicates the current position within a scrollable range. The thumb
 *          size is proportional to the visible area relative to the total content,
 *          and its position reflects the current scroll offset.
 * 
 * The scrollbar automatically calculates thumb dimensions and position based on
 * the provided scroll values (current, minimum, maximum). It is designed to be
 * used alongside scrollable content like menus or lists to provide visual
 * feedback about scroll state.
 * 
 * @note This class is marked as final and cannot be inherited from.
 * 
 * @see Widget
 * @see menu_options_t
 */
class ScrollBar final : public Widget
{
public:
    /**
     * @brief Constructs a ScrollBar with specified position and dimensions
     * @param options Pointer to menu options configuration structure
     * @param x X coordinate position of the scrollbar on screen
     * @param y Y coordinate position of the scrollbar on screen
     * @param width Width of the scrollbar in pixels
     * @param height Height of the scrollbar in pixels
     * 
     * @pre options must not be nullptr and must remain valid for the scrollbar's lifetime
     * @pre width and height must be greater than 0
     * @pre x and y must be valid screen coordinates
     * @post ScrollBar is initialized with the specified geometry and ready for use
     * 
     * @note The scrollbar does not take ownership of the options structure and
     *       assumes it remains valid throughout the scrollbar's lifetime.
     */
    ScrollBar(const menu_options_t *options, size_t x, size_t y, size_t width, size_t height);

    /**
     * @brief Renders the scrollbar to the screen
     * @details Overrides the base Widget render method to draw the scrollbar track
     *          and thumb. The appearance is determined by the current scroll state
     *          and the menu options configuration.
     * 
     * @pre u8g2 display context must be initialized and ready for drawing
     * @post Scrollbar's visual representation is drawn to the display buffer
     * 
     * @note This method is called during each frame's render cycle. The scrollbar
     *       track and thumb are drawn based on the current scroll values set by refresh().
     */
    void render() override;

    /**
     * @brief Updates the scrollbar state with new scroll values
     * @param value Current scroll position value (must be between min and max)
     * @param max Maximum scroll value (total content size)
     * @param min Minimum scroll value (default: 0, typically the start of content)
     * 
     * @pre value must be between min and max (inclusive)
     * @pre max must be greater than or equal to min
     * @post Scrollbar thumb position and size are recalculated based on new values
     * 
     * @details This method recalculates the thumb's height and vertical position
     *          based on the provided scroll range and current position. The thumb
     *          height represents the proportion of visible content to total content,
     *          while its position represents the current scroll offset within the range.
     * 
     * @note Call this method whenever the scroll state changes to keep the
     *       scrollbar visualization synchronized with the actual content position.
     */
    void refresh(size_t value, size_t max, size_t min = 0);

private:
    // Position and dimensions
    size_t m_x;         ///< X coordinate of the scrollbar's left edge
    size_t m_y;         ///< Y coordinate of the scrollbar's top edge
    size_t m_width;     ///< Width of the scrollbar track in pixels
    size_t m_height;    ///< Height of the scrollbar track in pixels

    // Scroll state values
    size_t m_value;     ///< Current scroll position within the range [m_min, m_max]
    size_t m_max;       ///< Maximum scroll value representing the end of content
    size_t m_min;       ///< Minimum scroll value representing the start of content

    // Calculated thumb properties (updated by refresh())
    size_t m_thumbHeight;   ///< Calculated height of the scroll thumb in pixels
    size_t m_thumbY;        ///< Calculated Y position of the scroll thumb relative to track
};