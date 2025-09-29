/**
 * @file ScreenSaver.h
 * @brief Animated screensaver implementation with vehicle effect
 * @details This header defines the ScreenSaver class which provides an animated
 *          vehicle screensaver that activates during periods of user inactivity.
 *          The screensaver displays moving vehicles to prevent screen burn-in while
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
 * @brief Animated vehicle screensaver widget for system idle periods
 */
class ScreenSaver final : public Widget
{
  public:
    explicit ScreenSaver(menu_options_t *options);
    void Update(uint64_t dt) override;
    void Render() override;
    void OnButtonClicked(ButtonType button) override;

  private:
    /**
     * @enum VehicleType
     * @brief Types of available vehicles
     */
    enum class VehicleType
    {
        CAR,
        CONVERTABLE,
        SUV,
        LORRY,
        TRUCK
    };

    /**
     * @enum Direction
     * @brief Movement direction for vehicles
     */
    enum class Direction
    {
        LEFT,
        RIGHT
    };

    /**
     * @struct Vehicle
     * @brief Individual vehicle object for animation
     */
    struct Vehicle
    {
        int x;               // X position on screen
        int y;               // Y position on screen
        float speed;         // Movement speed
        VehicleType type;    // Type of vehicle
        Direction direction; // Movement direction
        bool active;         // Whether a vehicle is currently active
    };

    static constexpr int MAX_LEFT_VEHICLES = 2;
    static constexpr int MAX_RIGHT_VEHICLES = 2;
    static constexpr int MAX_VEHICLES = MAX_LEFT_VEHICLES + MAX_RIGHT_VEHICLES;
    static constexpr int VEHICLE_SPAWN_DELAY = 2500; // milliseconds
    static constexpr float MIN_SPEED = 1.0f;
    static constexpr float MAX_SPEED = 2.0f;
    static constexpr int MIN_SAME_DIRECTION_DISTANCE = 48; // 32 + 16 pixels
    static constexpr int MAX_SAME_DIRECTION_DISTANCE = 64; // 32 + 32 pixels

    menu_options_t *m_options;
    uint64_t m_animationCounter;
    uint64_t m_lastSpawnTime;

    std::vector<Vehicle> m_vehicles;
    int m_leftVehicleCount;
    int m_rightVehicleCount;
    int m_sceneOffsetX = 0;
    int m_sceneOffsetY = 0;
    uint64_t m_sceneShiftTimer = 0;

    /**
     * @brief Initialize vehicle system
     */
    void initVehicles();

    /**
     * @brief Spawn a new vehicle if conditions are met
     */
    void trySpawnVehicle();

    /**
     * @brief Get a random vehicle type
     * @return Random VehicleType
     */
    static VehicleType getRandomVehicleType();

    /**
     * @brief Get a random direction with constraint checking
     * @return Direction for new vehicle
     */
    static Direction getRandomDirection();

    /**
     * @brief Draw a vehicle at a specified position
     * @param vehicle Vehicle to draw
     */
    void drawVehicle(const Vehicle &vehicle) const;

    /**
     * @brief Draw a bitmap with transparency (black pixels are transparent)
     * @param x X position
     * @param y Y position
     * @param width Bitmap width
     * @param height Bitmap height
     * @param bitmap Bitmap data
     */
    void drawTransparentBitmap(int x, int y, int width, int height, const unsigned char *bitmap) const;

    /**
     * @brief Get vehicle bitmap data
     * @param type Vehicle type
     * @param direction Movement direction
     * @param width Output parameter for bitmap width
     * @param height Output parameter for bitmap height
     * @return Pointer to bitmap data
     */
    static const unsigned char *getVehicleBitmap(VehicleType type, Direction direction, int &width, int &height);

    /**
     * @brief Check if there's enough distance to spawn a vehicle in a specific direction
     * @param direction Direction to check
     * @return true if spawning is allowed
     */
    bool canSpawnInDirection(Direction direction) const;
};