/**
 * @file Widget.h
 * @brief Base widget class for UI components in the INSA system
 * @details This header defines the Widget base class that serves as the foundation
 *          for all UI components in the system. It provides a standardized interface
 *          for rendering, updating, and handling user input using the u8g2 graphics library.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "u8g2.h"

#include "common/Common.h"

/**
 * @class Widget
 * @brief Base class for UI widgets that can be rendered and interact with user input
 * @details This abstract base class provides a common interface for all widgets in the system.
 *          It manages the u8g2 display context and defines the core methods that all widgets
 *          must implement or can override. The class follows the template method pattern,
 *          allowing derived classes to customize behavior while maintaining a consistent
 *          interface for the UI system.
 *
 * @note All widgets should inherit from this class to ensure compatibility with
 *       the UI management system.
 *
 * @see u8g2_t
 * @see ButtonType
 */
class Widget
{
  public:
    /**
     * @brief Constructs a widget with the given u8g2 display context
     * @param u8g2 Pointer to the u8g2 display context used for rendering operations
     *
     * @pre u8g2 must not be nullptr
     * @post Widget is initialized with the provided display context
     *
     * @note The widget does not take ownership of the u8g2 context and assumes
     *       it remains valid for the lifetime of the widget.
     */
    explicit Widget(u8g2_t *u8g2);

    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes
     * @details Ensures that derived class destructors are called correctly when
     *          a widget is destroyed through a base class pointer.
     */
    virtual ~Widget() = default;

    /**
     * @brief Called when the widget becomes active or enters the foreground
     * @details This method is invoked when the widget transitions from inactive
     *          to active state, such as when it becomes the current screen or
     *          gains focus. Derived classes can override this method to perform
     *          initialization tasks, reset state, or prepare for user interaction.
     *
     * @note The base implementation is empty, allowing derived classes to override
     *       only if entry behavior is needed.
     * @note This method is typically called by the UI management system during
     *       screen transitions or focus changes.
     */
    virtual void onEnter();

    /**
     * @brief Called when the widget is temporarily paused or loses focus
     * @details This method is invoked when the widget needs to suspend its
     *          operations temporarily, such as when another widget takes focus
     *          or the system enters a paused state. Derived classes can override
     *          this method to pause animations, save state, or reduce resource usage.
     *
     * @note The base implementation is empty, allowing derived classes to override
     *       only if pause behavior is needed.
     * @note The widget should be prepared to resume from this state when resume() is called.
     */
    virtual void onPause();

    /**
     * @brief Called when the widget resumes from a paused state
     * @details This method is invoked when the widget transitions from paused
     *          to active state, typically after a previous pause() call. Derived
     *          classes can override this method to restore animations, reload
     *          resources, or continue interrupted operations.
     *
     * @note The base implementation is empty, allowing derived classes to override
     *       only if resume behavior is needed.
     * @note This method should restore the widget to the state it was in before pause() was called.
     */
    virtual void onResume();

    /**
     * @brief Called when the widget is being destroyed or exits the system
     * @details This method is invoked when the widget is about to be removed
     *          from the system or transitions to an inactive state permanently.
     *          Derived classes can override this method to perform cleanup tasks,
     *          save final state, or release resources that are not automatically freed.
     *
     * @note The base implementation is empty, allowing derived classes to override
     *       only if onExit behavior is needed.
     * @note This method is called before the widget's destructor and provides
     *       an opportunity for controlled shutdown of widget-specific resources.
     */
    virtual void onExit();

    /**
     * @brief Updates the widget's internal state based on elapsed time
     * @param dt Delta time in milliseconds since the last update call
     *
     * @details This method is called once per frame by the UI system to handle
     *          animations, timers, state transitions, or other time-dependent behavior.
     *          The base implementation is empty, allowing derived classes to override
     *          only if time-based updates are needed.
     *
     * @note Override this method in derived classes to implement time-based behavior
     *       such as animations, blinking effects, or timeout handling.
     */
    virtual void Update(uint64_t dt);

    /**
     * @brief Renders the widget to the display
     * @details This method should be overridden by derived classes to implement
     *          their specific rendering logic using the u8g2 display context.
     *          The base implementation is empty, requiring derived classes to
     *          provide their own rendering code.
     *
     * @pre u8g2 context must be initialized and ready for drawing operations
     * @post Widget's visual representation is drawn to the display buffer
     *
     * @note This method is called during the rendering phase of each frame.
     *       Derived classes should use the u8g2 member variable to perform
     *       drawing operations.
     */
    virtual void Render();

    /**
     * @brief Handles button press events
     * @param button The type of button that was pressed
     *
     * @details This method is called when a button press event occurs and allows
     *          the widget to respond to user input. The base implementation is empty,
     *          allowing derived classes to override only if input handling is needed.
     *
     * @note Override this method in derived classes to implement button-specific
     *       behavior such as navigation, selection, or state changes.
     *
     * @see ButtonType for available button types
     */
    virtual void OnButtonClicked(ButtonType button);

    /**
     * @brief Returns the name of this widget for diagnostic purposes
     * @return A string identifying the widget type
     *
     * @details This method returns a human-readable name for the widget which
     *          is used for logging and diagnostic events. Derived classes should
     *          override this method to return their specific screen/widget name.
     *
     * @note The base implementation returns "Widget". Override in derived classes
     *       to provide meaningful screen names for diagnostics.
     */
    virtual const char *getName() const;

  protected:
    /**
     * @brief Pointer to the u8g2 display context used for rendering operations
     * @details This member provides access to the u8g2 graphics library functions
     *          for drawing text, shapes, bitmaps, and other UI elements. It is
     *          initialized during construction and remains valid for the widget's lifetime.
     *
     * @note This member is protected to allow derived classes direct access while
     *       preventing external modification. Derived classes should use this
     *       context for all rendering operations.
     *
     * @warning Do not modify or delete this pointer. The widget does not own
     *          the u8g2 context and assumes it is managed externally.
     */
    u8g2_t *u8g2;
};