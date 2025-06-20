/**
 * @file setup.h
 * @brief System initialization and main loop declarations for embedded application
 * @details This header defines the core system initialization and main loop functions
 *          required for embedded ESP32 applications. It provides the essential entry
 *          points for hardware setup, system configuration, and continuous operation
 *          management following standard embedded system patterns.
 * @author System Control Team
 * @date 2025-06-20
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initializes all system components and hardware peripherals
     * 
     * @details This function performs complete system initialization including:
     *          - Hardware peripheral configuration (GPIO, I2C, SPI, etc.)
     *          - Display system initialization
     *          - Button and input device setup
     *          - Communication subsystem initialization
     *          - Memory and storage system preparation
     *          - Application-specific component initialization
     * 
     * This function is called once during system startup before entering
     * the main application loop. It ensures all required subsystems are
     * properly configured and ready for operation.
     * 
     * @pre System must be in a clean startup state
     * @post All system components are initialized and ready for operation
     * 
     * @note This function must complete successfully before loop() is called
     * @note Any initialization failures should be handled gracefully with
     *       appropriate error reporting or system recovery
     * 
     * @see loop() for the main application execution function
     */
    void setup(void);

    /**
     * @brief Main application execution loop for continuous operation
     * 
     * @details This function contains the main application logic that executes
     *          continuously after system initialization. It typically handles:
     *          - User input processing and event handling
     *          - Display updates and rendering operations
     *          - System state management and transitions
     *          - Background tasks and periodic operations
     *          - Communication handling and data processing
     *          - Power management and system monitoring
     * 
     * The loop function is called repeatedly in an infinite cycle, providing
     * the main execution context for the embedded application. It should be
     * designed to execute efficiently without blocking to maintain system
     * responsiveness.
     * 
     * @note This function runs continuously and should not block indefinitely
     * @note All operations within this function should be non-blocking or
     *       use appropriate task scheduling for time-consuming operations
     * @note The function should handle all runtime errors gracefully
     * 
     * @see setup() for system initialization before loop execution
     */
    void loop(void);
#ifdef __cplusplus
}
#endif