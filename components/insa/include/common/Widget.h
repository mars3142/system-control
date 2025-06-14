#pragma once

#include "u8g2.h"

/**
 * Base class for UI widgets that can be rendered and interact with user input.
 * This class provides a common interface for all widgets in the system.
 */
class Widget
{
public:
    /**
     * Constructs a widget with the given u8g2 display context.
     * @param u8g2 Pointer to the u8g2 display context used for rendering
     */
    explicit Widget(u8g2_t *u8g2);

    /**
     * Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~Widget() = default;

    /**
     * Updates the widget's internal state based on elapsed time.
     * This method is called once per frame to handle animations,
     * timers, or other time-dependent behavior.
     * @param dt Delta time in milliseconds since last update
     */
    virtual void update(uint64_t dt);

    /**
     * Renders the widget to the display.
     * This method should be overridden by derived classes to implement
     * their specific rendering logic using the u8g2 display context.
     */
    virtual void render();

    /**
     * Handles button click events.
     * This method is called when a button is pressed and allows
     * the widget to respond to user input.
     * @param button The identifier of the button that was clicked
     */
    virtual void onButtonClicked(uint8_t button);

protected:
    /**
     * Pointer to the u8g2 display context used for rendering operations.
     * This provides access to drawing functions for text, graphics, and other UI elements.
     */
    u8g2_t *u8g2;
};