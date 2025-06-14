/**
 * @file SplashScreen.h
 * @brief Application splash screen implementation for startup presentation
 * @details This header defines the SplashScreen class which provides the initial
 *          visual presentation when the application starts. It serves as a loading
 *          screen that displays branding information, initialization progress, and
 *          provides visual feedback during the application startup sequence.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"

/**
 * @class SplashScreen
 * @brief Application startup screen widget with branding and initialization feedback
 * @details This final class inherits from Widget and represents the initial screen
 *          displayed when the application starts. It serves multiple purposes including
 *          brand presentation, system initialization feedback, and smooth transition
 *          preparation to the main application interface.
 * 
 * The SplashScreen class provides:
 * - Brand identity display (logos, company information, product name)
 * - System initialization progress indication
 * - Smooth animations and visual effects for professional appearance
 * - Automatic transition timing to main menu after initialization
 * - Error indication if initialization fails
 * 
 * Key features include:
 * - Time-based automatic progression to main menu
 * - Animated elements (fade-in effects, progress indicators)
 * - Version information display
 * - Loading status messages
 * - Graceful handling of initialization delays
 * 
 * The splash screen typically displays for a minimum duration to ensure users
 * can read branding information, even if system initialization completes quickly.
 * It automatically transitions to the main menu once both the minimum display
 * time and system initialization are complete.
 * 
 * @note This class is marked as final and cannot be inherited from.
 * @note The splash screen does not handle user input - it operates on timing
 *       and system state rather than user interaction.
 * 
 * @see Widget for base widget functionality
 * @see menu_options_t for configuration structure
 * @see MainMenu for the target transition screen
 */
class SplashScreen final : public Widget
{
public:
    /**
     * @brief Constructs the splash screen with specified configuration
     * @param options Pointer to menu options configuration structure
     * 
     * @pre options must not be nullptr and must remain valid for the splash screen's lifetime
     * @pre options->u8g2 must be initialized and ready for graphics operations
     * @pre Screen transition callbacks in options must be properly configured
     * @post SplashScreen is initialized and ready to display startup sequence
     * 
     * @details The constructor initializes the splash screen by:
     *          - Setting up the initial display state and animations
     *          - Preparing branding elements (logos, text, version info)
     *          - Initializing timing controls for minimum display duration
     *          - Configuring transition parameters for smooth progression
     *          - Loading any required graphics resources or assets
     * 
     * The splash screen prepares all visual elements during construction to
     * ensure smooth rendering performance during the critical startup phase.
     * It also establishes the timing framework for controlling display duration
     * and transition timing.
     * 
     * @note The splash screen does not take ownership of the options structure
     *       and assumes it remains valid throughout the screen's lifetime.
     * @note Graphics resources are loaded during construction, so any required
     *       assets must be available at initialization time.
     * 
     * @see Widget::Widget for base class construction details
     */
    explicit SplashScreen(menu_options_t *options);
    
    /**
     * @brief Updates the splash screen state, animations, and timing logic
     * @param dt Delta time in milliseconds since the last update call
     * 
     * @details Overrides the base Widget update method to handle splash screen-specific
     *          logic including:
     *          - Animation progression (fade effects, transitions, progress indicators)
     *          - Timing control for minimum display duration
     *          - System initialization status monitoring
     *          - Automatic transition preparation to main menu
     *          - Loading progress updates and status message changes
     * 
     * The update method manages the splash screen's lifecycle by tracking
     * elapsed time and system readiness state. It ensures the splash screen
     * remains visible for a minimum duration while also monitoring system
     * initialization completion. Once both conditions are met, it initiates
     * the transition to the main application interface.
     * 
     * Animation updates include:
     * - Fade-in effects for branding elements
     * - Progress bar or spinner animations
     * - Text transitions for status messages
     * - Smooth preparation for screen transition
     * 
     * @note This method is called every frame during the splash screen display
     *       and must be efficient to maintain smooth visual presentation.
     * @note The method automatically handles transition to the main menu when
     *       appropriate, using the callback functions provided in m_options.
     * 
     * @see Widget::update for the base update interface
     */
    void update(uint64_t dt) override;
    
    /**
     * @brief Renders the splash screen visual elements to the display
     * 
     * @details Overrides the base Widget render method to draw all splash screen
     *          elements including branding, status information, and visual effects.
     *          The rendering includes:
     *          - Company/product logos and branding elements
     *          - Application name and version information
     *          - Loading progress indicators (progress bars, spinners, etc.)
     *          - Status messages indicating initialization progress
     *          - Background graphics and visual effects
     * 
     * The render method creates a professional, polished appearance that
     * reinforces brand identity while providing useful feedback about the
     * application startup process. All elements are positioned and styled
     * to create a cohesive, attractive presentation.
     * 
     * Rendering features include:
     * - Centered layout with balanced visual hierarchy
     * - Smooth animations and transitions
     * - Consistent typography and color scheme
     * - Progress feedback elements
     * - Error indication if initialization problems occur
     * 
     * @pre u8g2 display context must be initialized and ready for drawing
     * @post All splash screen visual elements are drawn to the display buffer
     * 
     * @note This method is called every frame and must be efficient to maintain
     *       smooth visual presentation during the startup sequence.
     * @note The visual design should be consistent with the overall application
     *       theme and branding guidelines.
     * 
     * @see Widget::render for the base render interface
     */
    void render() override;

private:
    /**
     * @brief Pointer to menu options configuration structure
     * @details Stores a reference to the menu configuration passed during construction.
     *          This provides access to the display context for rendering operations
     *          and screen transition callbacks for automatic progression to the main menu.
     * 
     * The configuration enables:
     * - Display context (u8g2) for graphics rendering operations
     * - Screen transition callbacks for automatic progression to main menu
     * - System integration for initialization status monitoring
     * 
     * The splash screen uses the setScreen callback to automatically transition
     * to the main menu once the startup sequence is complete. This ensures a
     * seamless user experience from application launch to main interface.
     * 
     * @note This pointer is not owned by the SplashScreen and must remain valid
     *       throughout the screen's lifetime. It is managed by the application framework.
     * 
     * @warning Must not be modified after construction as it contains critical
     *          system callbacks required for proper application flow.
     */
    menu_options_t *m_options;
};