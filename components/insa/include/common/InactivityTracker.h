/**
 * @file InactivityTracker.h
 * @brief Inactivity tracking system for monitoring user interaction timeouts
 * @details This header defines the InactivityTracker class which monitors user
 *          activity and triggers timeout callbacks when the system remains inactive
 *          for a specified duration. It provides essential functionality for power
 *          management, screen savers, and automatic system state transitions.
 * @author System Control Team
 * @date 2025-06-20
 */

#pragma once

#include <functional>
#include <stdint.h>

/**
 * @class InactivityTracker
 * @brief Activity monitoring class for detecting user inactivity periods
 * @details This class provides a robust mechanism for tracking user activity and
 *          triggering automatic actions when the system remains inactive for a
 *          configured timeout period. It is commonly used for implementing power
 *          saving features, automatic screen savers, session timeouts, and other
 *          time-based system behaviors.
 *
 * The InactivityTracker operates by:
 * - Continuously tracking elapsed time since the last user activity
 * - Comparing elapsed time against a configurable timeout threshold
 * - Executing a callback function when the timeout is reached
 * - Providing methods to reset the timer when activity is detected
 * - Supporting enable/disable functionality for dynamic control
 *
 * Key features include:
 * - Configurable timeout duration in milliseconds
 * - Custom callback function execution on timeout
 * - Activity reset capability for responsive user interaction
 * - Enable/disable control for conditional monitoring
 * - High-resolution timing support using 64-bit millisecond precision
 *
 * Common use cases:
 * - Screen saver activation after idle periods
 * - Automatic screen dimming or shutdown
 * - Session timeout management
 * - Power management and battery conservation
 * - User interface state transitions
 * - Security lockout after inactivity
 *
 * The class is designed to be lightweight and efficient, suitable for
 * real-time applications where precise timing and minimal overhead are important.
 *
 * @note This class requires regular update calls to function properly.
 * @note The timeout callback is executed once per timeout period and will
 *       not repeat until the tracker is reset and times out again.
 *
 * @see Widget for integration with UI components
 * @see Menu for menu timeout implementations
 */
class InactivityTracker
{
  public:
    /**
     * @brief Constructs an InactivityTracker with specified timeout and callback
     * @param timeoutMs Timeout duration in milliseconds before triggering callback
     * @param onTimeout Callback function to execute when timeout is reached
     *
     * @pre timeoutMs must be greater than 0 for meaningful timeout behavior
     * @pre onTimeout must be a valid callable function object
     * @post InactivityTracker is initialized, enabled, and ready for activity monitoring
     *
     * @details The constructor initializes the inactivity tracker with the specified
     *          timeout duration and callback function. The tracker starts in an enabled
     *          state with zero elapsed time, ready to begin monitoring user activity.
     *
     * The timeout callback function can perform any necessary actions when inactivity
     * is detected, such as:
     * - Activating screen savers or power saving modes
     * - Transitioning to different application states
     * - Logging inactivity events
     * - Triggering security lockouts
     * - Initiating automatic save operations
     *
     * @note The tracker begins monitoring immediately upon construction.
     * @note The callback function should be lightweight to avoid blocking
     *       the main application thread during timeout processing.
     *
     * Example usage:
     * @code
     * auto tracker = InactivityTracker(30000, []() {
     *     // Activate screen saver after 30 seconds of inactivity
     *     activateScreenSaver();
     * });
     * @endcode
     */
    InactivityTracker(uint64_t timeoutMs, std::function<void()> onTimeout);

    /**
     * @brief Updates the inactivity timer and checks for timeout conditions
     * @param dt Delta time in milliseconds since the last update call
     *
     * @details This method must be called regularly (typically every frame) to
     *          maintain accurate timing and timeout detection. It increments the
     *          elapsed time counter and triggers the timeout callback when the
     *          configured timeout duration is reached.
     *
     * The update process:
     * - Adds the delta time to the elapsed time counter (if enabled)
     * - Compares elapsed time against the configured timeout threshold
     * - Executes the timeout callback if the threshold is exceeded
     * - Continues monitoring until reset or disabled
     *
     * @note This method should be called consistently from the main application
     *       loop to ensure accurate timing behavior.
     * @note The timeout callback is executed only once per timeout period.
     * @note If the tracker is disabled, elapsed time is not updated.
     *
     * @see reset() to restart the inactivity timer
     * @see setEnabled() to control monitoring state
     */
    void update(uint64_t dt);

    /**
     * @brief Resets the inactivity timer to indicate recent user activity
     *
     * @details This method should be called whenever user activity is detected
     *          to restart the inactivity timeout period. It resets the elapsed
     *          time counter to zero, effectively extending the timeout deadline
     *          and preventing timeout callback execution until the full timeout
     *          duration elapses again without further resets.
     *
     * Common scenarios for calling reset():
     * - Button presses or key events
     * - Mouse movement or touch input
     * - Menu navigation or selection actions
     * - Any user interface interaction
     * - System activity that should extend the timeout
     *
     * @post Elapsed time is reset to zero, restarting the timeout period
     *
     * @note This method can be called at any time, even when the tracker
     *       is disabled, to prepare for future monitoring.
     * @note Frequent reset calls from active user interaction will prevent
     *       timeout callback execution, which is the intended behavior.
     *
     * Example usage:
     * @code
     * void onButtonPress() {
     *     tracker.reset(); // User activity detected, restart timeout
     *     // Handle button press...
     * }
     * @endcode
     */
    void reset();

    /**
     * @brief Enables or disables inactivity monitoring
     * @param enabled True to enable monitoring, false to disable
     *
     * @details This method controls whether the inactivity tracker actively
     *          monitors for timeouts. When disabled, the elapsed time counter
     *          is not updated during update() calls, effectively pausing the
     *          timeout detection without losing the current elapsed time state.
     *
     * Use cases for disabling:
     * - Temporary suspension during system operations
     * - Context-sensitive monitoring (disable in certain application states)
     * - Power management control (disable during low-power modes)
     * - User preference settings (allow users to disable timeouts)
     * - Development and debugging (disable for testing)
     *
     * When re-enabled, monitoring resumes from the current elapsed time state,
     * allowing for seamless pause/resume functionality.
     *
     * @post Monitoring state is updated according to the enabled parameter
     *
     * @note Disabling the tracker does not reset the elapsed time counter.
     * @note The timeout callback will not be executed while disabled, even
     *       if the timeout threshold would otherwise be exceeded.
     * @note Enabling/disabling can be done at any time during operation.
     *
     * Example usage:
     * @code
     * tracker.setEnabled(false); // Pause monitoring during critical operation
     * performCriticalOperation();
     * tracker.setEnabled(true);  // Resume monitoring after completion
     * @endcode
     */
    void setEnabled(bool enabled);

  private:
    uint64_t m_timeoutMs;              ///< Timeout duration in milliseconds before callback execution
    uint64_t m_elapsedTime;            ///< Current elapsed time since last reset in milliseconds
    bool m_enabled;                    ///< Flag indicating whether monitoring is currently active
    std::function<void()> m_onTimeout; ///< Callback function executed when timeout threshold is reached
};