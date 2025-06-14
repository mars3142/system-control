#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"

/**
 * @brief SplashScreen class represents the initial screen shown when the application starts
 * 
 * This class inherits from Widget and is responsible for displaying the splash screen
 * that typically shows application branding, loading status, or initialization messages.
 * It's marked as final to prevent further inheritance.
 */
class SplashScreen final : public Widget
{
  public:
    /**
     * @brief Constructs a new SplashScreen object
     * 
     * @param options Pointer to menu options configuration that controls
     *                the behavior and appearance of the splash screen
     */
    explicit SplashScreen(menu_options_t *options);
    
    /**
     * @brief Updates the splash screen state and animations
     * 
     * This method is called every frame to update the splash screen's
     * internal state, handle timing, animations, or transitions.
     * 
     * @param dt Delta time in milliseconds since the last update
     */
    void update(uint64_t dt) override;
    
    /**
     * @brief Renders the splash screen to the display
     * 
     * This method handles all the drawing operations for the splash screen,
     * including background, logos, text, and any visual effects.
     */
    void render() override;

  private:
    /**
     * @brief Pointer to menu options configuration
     * 
     * Stores the configuration options that control various aspects
     * of the splash screen's behavior and appearance.
     */
    menu_options_t *m_options;
};