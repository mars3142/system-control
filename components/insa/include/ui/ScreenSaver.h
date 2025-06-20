/**
 * @file ScreenSaver.h
 * @brief Animated screensaver implementation with starfield effect
 * @details This header defines the ScreenSaver class which provides an animated
 *          starfield screensaver that activates during periods of user inactivity.
 *          The screensaver displays moving stars to prevent screen burn-in while
 *          providing visual feedback that the system is still active and responsive.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"
#include <vector>

/**
 * @class ScreenSaver
 * @brief Animated starfield screensaver widget for system idle periods
 * @details This final class inherits from Widget and implements an interactive
 *          screensaver featuring an animated starfield effect. The screensaver
 *          activates automatically during periods of user inactivity to prevent
 *          screen burn-in while maintaining visual indication of system activity.
 * 
 * The ScreenSaver class provides:
 * - Dynamic starfield animation with pseudo-3D depth effect
 * - Configurable star count and animation speed parameters
 * - Automatic activation during idle periods
 * - Immediate deactivation on any user input
 * - Smooth star movement and regeneration system
 * - Memory-efficient star management with object pooling
 * 
 * Key features include:
 * - 3D-style starfield with depth-based speed variation
 * - Continuous star recycling for infinite animation
 * - Responsive user input handling for immediate exit
 * - Optimized rendering for smooth animation performance
 * - Configurable animation parameters for different visual effects
 * 
 * The starfield effect simulates movement through space by moving stars
 * from the center outward at speeds proportional to their depth (z-coordinate).
 * Stars that move beyond the screen boundaries are automatically recycled
 * to new random positions, creating an infinite scrolling effect.
 * 
 * @note This class is marked as final and cannot be inherited from.
 * @note The screensaver automatically exits on any button press, returning
 *       control to the previous screen or main menu interface.
 * 
 * @see Widget for base widget functionality
 * @see menu_options_t for configuration structure
 * @see InactivityTracker for automatic screensaver activation
 */
class ScreenSaver final : public Widget
{
  public:
    /**
     * @brief Constructs the screensaver with specified configuration
     * @param options Pointer to menu options configuration structure
     * 
     * @pre options must not be nullptr and must remain valid for the screensaver's lifetime
     * @pre options->u8g2 must be initialized and ready for graphics operations
     * @pre Screen transition callbacks in options must be properly configured
     * @post ScreenSaver is initialized with starfield ready for animation
     * 
     * @details The constructor initializes the screensaver by:
     *          - Setting up the star field with initial random positions
     *          - Configuring animation timing and speed parameters
     *          - Preparing graphics context for efficient rendering
     *          - Initializing the animation counter for smooth timing
     *          - Allocating and positioning all star objects
     * 
     * During initialization, all stars are placed at random positions within
     * the 3D space defined by Z_NEAR and Z_FAR constants. Each star receives
     * random x, y coordinates and a z-depth that determines its movement speed
     * and visual appearance. The animation system is prepared for immediate
     * activation when the screensaver becomes active.
     * 
     * @note The screensaver does not take ownership of the options structure
     *       and assumes it remains valid throughout the screensaver's lifetime.
     * @note Initial star positions are randomized to create immediate visual
     *       interest when the screensaver first activates.
     * 
     * @see Widget::Widget for base class construction details
     * @see initStars for star field initialization process
     */
    explicit ScreenSaver(menu_options_t *options);

    /**
     * @brief Updates the screensaver animation and star positions
     * @param dt Delta time in milliseconds since the last update call
     * 
     * @details Overrides the base Widget update method to handle screensaver-specific
     *          animation logic including:
     *          - Advancing the animation counter for smooth timing control
     *          - Updating individual star positions based on their speed and depth
     *          - Moving stars outward from center based on their z-coordinate
     *          - Recycling stars that move beyond screen boundaries
     *          - Maintaining consistent animation frame rate regardless of system load
     * 
     * The update method implements the core starfield animation by:
     * - Incrementing each star's position based on its depth-determined speed
     * - Checking boundary conditions for stars moving off-screen
     * - Resetting off-screen stars to new random positions near the center
     * - Applying speed multipliers for smooth, consistent motion
     * - Managing the animation counter for timing-dependent effects
     * 
     * Star movement calculation:
     * - Stars closer to the camera (smaller z values) move faster
     * - Movement speed is inversely proportional to z-coordinate
     * - Stars maintain consistent outward direction from screen center
     * - Boundary checking ensures smooth recycling without visual gaps
     * 
     * @note This method is called every frame while the screensaver is active
     *       and must be efficient to maintain smooth 60+ FPS animation.
     * @note The animation continues indefinitely until user input is received.
     * 
     * @see Widget::update for the base update interface
     * @see resetStar for star recycling implementation
     */
    void update(uint64_t dt) override;
    
    /**
     * @brief Renders the animated starfield to the display
     * 
     * @details Overrides the base Widget render method to draw the animated starfield
     *          effect. The rendering process includes:
     *          - Clearing the display buffer for clean animation frames
     *          - Calculating screen positions for each star based on 3D coordinates
     *          - Applying perspective projection to simulate depth
     *          - Drawing stars with size/brightness based on distance
     *          - Optimizing drawing operations for smooth frame rates
     * 
     * The render method creates a convincing 3D starfield effect by:
     * - Converting 3D star coordinates to 2D screen positions
     * - Scaling star positions based on perspective projection
     * - Varying star appearance (size, brightness) based on depth
     * - Drawing only stars within the visible screen area
     * - Using efficient drawing primitives for optimal performance
     * 
     * Rendering optimizations include:
     * - Culling stars outside the visible area
     * - Using simple pixel/point drawing for maximum speed
     * - Minimizing graphics context switches
     * - Batching drawing operations where possible
     * 
     * The visual effect simulates movement through a star field by:
     * - Making distant stars appear smaller and dimmer
     * - Scaling star positions relative to screen center
     * - Creating smooth motion blur effects for fast-moving stars
     * 
     * @pre u8g2 display context must be initialized and ready for drawing
     * @post Starfield animation frame is drawn to the display buffer
     * 
     * @note This method is called every frame and must be highly optimized
     *       to maintain smooth animation performance on embedded systems.
     * @note The visual design provides an engaging screensaver that clearly
     *       indicates system activity while preventing screen burn-in.
     * 
     * @see Widget::render for the base render interface
     */
    void render() override;
    
    /**
     * @brief Handles user input to exit the screensaver immediately
     * @param button The type of button that was pressed by the user
     * 
     * @details Overrides the base Widget button handling to provide immediate
     *          screensaver exit functionality. Any button press while the screensaver
     *          is active will:
     *          - Immediately terminate the screensaver animation
     *          - Return to the previous screen or main menu
     *          - Reset any inactivity timers to prevent immediate reactivation
     *          - Ensure responsive system behavior for user interaction
     * 
     * The method handles all button types uniformly since the screensaver should
     * exit on any user input regardless of the specific button pressed. This
     * ensures maximum responsiveness and intuitive behavior - users expect any
     * interaction to wake the system from screensaver mode.
     * 
     * Button handling includes:
     * - Immediate screensaver termination regardless of button type
     * - Automatic transition back to the previous active screen
     * - Inactivity timer reset to prevent immediate screensaver reactivation
     * - Proper state cleanup to ensure system stability
     * 
     * @note This method ensures the screensaver never interferes with normal
     *       system operation - any user input immediately restores full functionality.
     * @note The screensaver exit process is designed to be instantaneous to
     *       provide the most responsive user experience possible.
     * 
     * @see Widget::onButtonClicked for the base input handling interface
     * @see ButtonType for available button input types
     */
    void onButtonClicked(ButtonType button) override;

  private:
    /**
     * @struct Star
     * @brief Individual star object structure for starfield animation
     * @details Defines the properties and state of a single star in the animated
     *          starfield. Each star maintains its position in 3D space and movement
     *          characteristics needed for realistic animation and perspective effects.
     * 
     * The Star structure contains:
     * - 3D spatial coordinates (x, y, z) for position tracking
     * - Individual speed multiplier for varied animation effects
     * - All data needed for perspective projection and movement calculation
     * 
     * Star coordinate system:
     * - x, y: Screen-relative coordinates (can be negative for off-screen positioning)
     * - z: Depth coordinate determining speed and perspective (Z_NEAR to Z_FAR range)
     * - speed: Individual multiplier for varied star movement rates
     * 
     * @note This structure is designed for efficient memory usage and fast
     *       mathematical operations during animation updates.
     * @note All coordinates use float precision for smooth animation interpolation.
     */
    struct Star
    {
        float x;        ///< Horizontal position coordinate (screen-relative)
        float y;        ///< Vertical position coordinate (screen-relative)  
        float z;        ///< Depth coordinate (determines speed and perspective)
        float speed;    ///< Individual speed multiplier for animation variation
    };

    /**
     * @brief Pointer to menu options configuration structure
     * @details Stores a reference to the menu configuration passed during construction.
     *          Provides access to the display context for rendering operations and
     *          screen transition callbacks for exiting the screensaver on user input.
     * 
     * The configuration enables:
     * - Display context (u8g2) for starfield graphics rendering
     * - Screen transition callbacks for returning to previous screen
     * - System integration for proper screensaver lifecycle management
     * 
     * @note This pointer is not owned by the ScreenSaver and must remain valid
     *       throughout the screensaver's lifetime.
     */
    menu_options_t *m_options;
    
    /**
     * @brief Animation timing counter for smooth frame rate control
     * @details Tracks elapsed time for animation timing and frame rate calculations.
     *          Used to ensure consistent star movement regardless of system load
     *          variations and to provide smooth interpolation between animation frames.
     * 
     * The counter enables:
     * - Frame rate independent animation timing
     * - Smooth interpolation for fluid star movement
     * - Consistent animation speed across different hardware platforms
     * - Precise timing control for animation effects
     */
    uint64_t m_animationCounter;
    
    /**
     * @brief Vector container for all star objects in the starfield
     * @details Manages the collection of Star objects that comprise the animated
     *          starfield effect. The vector provides efficient storage and iteration
     *          for the star animation system while maintaining good cache locality
     *          for optimal performance during update and render operations.
     * 
     * The vector contains:
     * - Fixed number of Star objects (NUM_STARS) allocated at initialization
     * - Contiguous memory layout for efficient iteration during animation
     * - Dynamic management capabilities for potential future enhancements
     * 
     * @note The vector size is fixed at construction to avoid memory allocations
     *       during animation, ensuring consistent frame timing performance.
     */
    std::vector<Star> m_stars;

    /**
     * @brief Total number of stars in the animated starfield
     * @details Defines the constant number of star objects that will be created
     *          and animated in the starfield effect. This value balances visual
     *          richness with performance requirements for smooth animation.
     * 
     * The star count affects:
     * - Visual density and richness of the starfield effect  
     * - Performance requirements for update and rendering operations
     * - Memory usage for star object storage
     * - Overall visual impact of the screensaver
     * 
     * @note This value is tuned for optimal performance on target hardware
     *       while providing an engaging visual effect.
     */
    static constexpr int NUM_STARS = 10;
    
    /**
     * @brief Global speed multiplier for star animation
     * @details Controls the overall speed of star movement in the starfield animation.
     *          This multiplier is applied to all star movement calculations to provide
     *          consistent, smooth animation at an appropriate visual speed.
     * 
     * The speed multiplier affects:
     * - Overall pace of the starfield animation
     * - Visual impact and engagement level of the screensaver
     * - Performance requirements for smooth animation
     * - User perception of system responsiveness
     * 
     * @note This value is carefully tuned to provide engaging animation without
     *       being distracting or causing motion sickness effects.
     */
    static constexpr float SPEED_MULTIPLIER = 0.02f;
    
    /**
     * @brief Near clipping plane for 3D starfield depth range
     * @details Defines the closest distance (minimum z-coordinate) at which stars
     *          can exist in the 3D starfield space. Stars closer than this distance
     *          are considered too close to the viewer and are recycled to new positions.
     * 
     * The near plane affects:
     * - Minimum depth for star positioning and recycling
     * - Perspective calculation range for realistic depth effects
     * - Star recycling triggers for continuous animation
     * - Visual depth range of the starfield effect
     * 
     * @note This value works with Z_FAR to define the 3D space within which
     *       stars exist and animate, creating realistic depth perception.
     */
    static constexpr float Z_NEAR = 0.1f;
    
    /**
     * @brief Far clipping plane for 3D starfield depth range  
     * @details Defines the farthest distance (maximum z-coordinate) at which stars
     *          can exist in the 3D starfield space. This establishes the back
     *          boundary of the starfield volume and affects initial star placement.
     * 
     * The far plane affects:
     * - Maximum depth for initial star positioning
     * - Perspective calculation range for depth effects
     * - Visual depth range and scale of the starfield
     * - Initial star placement during system initialization
     * 
     * @note This value works with Z_NEAR to create a realistic 3D space
     *       that provides convincing depth perception in the starfield animation.
     */
    static constexpr float Z_FAR = 10.0f;

    /**
     * @brief Initializes all stars with random positions and properties
     * @details Private helper method that sets up the initial starfield by placing
     *          all stars at random positions within the defined 3D space. Called
     *          during construction to prepare the starfield for immediate animation.
     * 
     * The initialization process:
     * - Assigns random x, y coordinates within screen boundaries
     * - Sets random z-depth within the Z_NEAR to Z_FAR range
     * - Configures individual speed multipliers for animation variation
     * - Ensures even distribution of stars throughout the 3D volume
     * 
     * Random placement creates:
     * - Natural, non-uniform star distribution for realistic appearance
     * - Varied star depths for convincing 3D perspective effects
     * - Immediate visual interest when screensaver first activates
     * - Foundation for smooth continuous animation
     * 
     * @note This method is called only once during construction to establish
     *       the initial starfield state before animation begins.
     * 
     * @see resetStar for individual star repositioning during animation
     */
    void initStars();
    
    /**
     * @brief Resets a single star to a new random position for recycling
     * @param star Reference to the Star object to be reset and repositioned
     * 
     * @details Private helper method that recycles individual stars that have
     *          moved beyond the visible screen boundaries. This enables infinite
     *          starfield animation by continuously introducing new stars while
     *          removing those that are no longer visible.
     * 
     * The reset process:
     * - Places the star at a new random position near the screen center
     * - Assigns a new random depth (z-coordinate) for varied movement speed
     * - Configures new speed multiplier for animation variation
     * - Ensures smooth transition without visual discontinuities
     * 
     * Star recycling maintains:
     * - Continuous starfield animation without visual gaps
     * - Consistent star count throughout animation lifecycle
     * - Varied star properties for natural, non-repetitive effects
     * - Efficient memory usage through object reuse
     * 
     * @pre star parameter must be a valid Star object reference
     * @post star object is repositioned with new random properties ready for animation
     * 
     * @note This method is called frequently during animation as stars move
     *       off-screen and must be efficient to maintain smooth frame rates.
     * @note The repositioning algorithm ensures stars appear to emerge naturally
     *       from the center of the starfield for convincing 3D movement effects.
     */
    void resetStar(Star &star);
};