/**
 * @file Common.h
 * @brief Common utility functions and window management for application framework
 * @details This header defines common utility functions that are shared across
 *          the application framework. It provides essential functionality for
 *          window creation and management, serving as a bridge between the
 *          application layer and the underlying windowing system.
 * @author System Control Team
 * @date 2025-06-20
 */

#pragma once

#include "model/Window.h"

/**
 * @brief Creates a new window with specified title and dimensions
 * @param title Null-terminated string containing the window title text
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @return Pointer to the newly created Window object, or nullptr on failure
 * 
 * @pre title must not be nullptr and should contain valid display text
 * @pre width and height must be positive values within system display limits
 * @post A new Window object is allocated and initialized with the specified parameters
 * @post The returned Window pointer is ready for use with window management functions
 * 
 * @details This function creates a new Window instance with the specified
 *          title and dimensions. It handles the underlying window system
 *          initialization, memory allocation, and setup required to create
 *          a functional window object.
 * 
 * The window creation process includes:
 * - Memory allocation for the Window structure
 * - Initialization of window properties (title, dimensions, state)
 * - Registration with the window management system
 * - Setup of default window behavior and event handling
 * 
 * The returned window pointer can be used with other window management
 * functions to display content, handle events, and manage the window
 * lifecycle. The caller is responsible for properly managing the window
 * lifetime and ensuring proper cleanup when the window is no longer needed.
 * 
 * @note The returned pointer must be properly managed by the caller
 * @note Window resources should be freed when no longer needed
 * @note The title string is copied internally and can be safely modified
 *       or freed after this function returns
 * 
 * @see Window class for window object interface and methods
 * 
 * Example usage:
 * @code
 * auto* window = CreateWindow("System Control", 800, 600);
 * if (window != nullptr) {
 *     // Use the window...
 *     // Clean up when done
 * }
 * @endcode
 */
auto CreateWindow(const char *title, int width, int height) -> Window *;