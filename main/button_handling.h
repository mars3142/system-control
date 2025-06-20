/**
 * @file button_handling.h
 * @brief Button input handling system for user interface interaction
 * @details This header defines the button handling subsystem which manages
 *          hardware button inputs, debouncing, interrupt processing, and
 *          event queue management. It provides a robust foundation for
 *          reliable user input processing in embedded applications.
 * @author System Control Team
 * @date 2025-06-20
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initializes the button handling subsystem and configures hardware
     * 
     * @details This function sets up the complete button handling infrastructure:
     *          - GPIO configuration for button input pins with pull-up resistors
     *          - Interrupt service routine installation for responsive input
     *          - Debouncing timer creation and configuration
     *          - FreeRTOS queue creation for button event buffering
     *          - Button state tracking structure initialization
     * 
     * The function configures all defined button pins to trigger interrupts
     * on both rising and falling edges, enabling detection of both press
     * and release events. Each button uses a dedicated timer for debouncing
     * to ensure reliable input processing even with mechanical switch bounce.
     * 
     * Button events are queued using FreeRTOS queues to ensure no input
     * events are lost during high system activity periods. The queue-based
     * approach also decouples interrupt handling from application processing.
     * 
     * @pre ESP32 GPIO and timer subsystems must be available and functional
     * @pre FreeRTOS must be running and queue services available
     * @post All button pins are configured and ready for input detection
     * @post Button event queue is created and ready for event processing
     * @post Interrupt handlers are installed and active
     * 
     * @note This function must be called during system initialization before
     *       any button input processing is expected
     * @note The function configures hardware-specific GPIO pins as defined
     *       in the project configuration
     * 
     * @see cleanup_buttons() for proper resource cleanup
     */
    void setup_buttons(void);

    /**
     * @brief Cleans up button handling resources and disables interrupts
     * 
     * @details This function performs complete cleanup of the button handling
     *          subsystem by:
     *          - Stopping and deleting all debouncing timers
     *          - Removing GPIO interrupt handlers from all button pins
     *          - Deleting the button event queue and freeing associated memory
     *          - Resetting button state tracking structures
     * 
     * This cleanup function ensures proper resource management and prevents
     * memory leaks when the button handling subsystem is no longer needed.
     * It can be called during system shutdown or when reconfiguring the
     * input handling subsystem.
     * 
     * @pre Button handling subsystem must have been previously initialized
     * @post All button-related interrupts are disabled and handlers removed
     * @post All timers are stopped and deleted, freeing system resources
     * @post Button event queue is deleted and memory is released
     * @post GPIO pins are returned to default state
     * 
     * @note This function should be called during system shutdown or when
     *       button handling is no longer required
     * @note After calling this function, setup_buttons() must be called
     *       again before button input can be processed
     * 
     * @see setup_buttons() for initialization of the button handling system
     */
    void cleanup_buttons(void);
#ifdef __cplusplus
}
#endif