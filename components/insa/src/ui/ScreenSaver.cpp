#include "ui/ScreenSaver.h"
#include <cstdlib>
#include "data/roads.h"
#include "data/vehicles.h"

ScreenSaver::ScreenSaver(menu_options_t *options)
    : Widget(options->u8g2),
      m_options(options),
      m_animationCounter(0),
      m_lastSpawnTime(0),
      m_leftVehicleCount(0),
      m_rightVehicleCount(0)
{
    initVehicles();
}

void ScreenSaver::initVehicles()
{
    m_vehicles.resize(MAX_VEHICLES);

    for (auto &vehicle : m_vehicles)
    {
        vehicle.active = false;
    }
}

void ScreenSaver::update(const uint64_t dt)
{
    m_animationCounter += dt;
    m_lastSpawnTime += dt;
    m_sceneShiftTimer += dt;

    // Shift entire scene every 30 seconds
    if (m_sceneShiftTimer > 30000)
    {
        m_sceneOffsetX = (random() % 7) - 3; // -3 to +3 pixels
        m_sceneOffsetY = (random() % 7) - 3; // -3 to +3 pixels
        m_sceneShiftTimer = 0;
    }

    // Try to spawn a new vehicle every few seconds
    if (m_lastSpawnTime > VEHICLE_SPAWN_DELAY)
    {
        trySpawnVehicle();
        m_lastSpawnTime = 0;
    }

    // Update vehicle positions
    if (m_animationCounter > 16) // ~60 FPS
    {
        m_animationCounter = 0;

        for (auto &vehicle : m_vehicles)
        {
            if (!vehicle.active)
                continue;

            // Move vehicle
            if (vehicle.direction == Direction::LEFT)
            {
                vehicle.x -= static_cast<int>(vehicle.speed);

                // Remove the vehicle if it goes off-screen
                if (vehicle.x <= -32) // Allow for largest vehicle width
                {
                    vehicle.active = false;
                    m_leftVehicleCount--;
                }
            }
            else // Direction::RIGHT
            {
                vehicle.x += static_cast<int>(vehicle.speed);

                // Remove the vehicle if it goes off-screen
                if (vehicle.x >= (u8g2->width + 32)) // Allow for largest vehicle width
                {
                    vehicle.active = false;
                    m_rightVehicleCount--;
                }
            }
        }
    }
}

void ScreenSaver::trySpawnVehicle()
{
    // Check if we can spawn a new vehicle
    int activeVehicles = 0;
    int availableSlot = -1;

    for (int i = 0; i < MAX_VEHICLES; i++)
    {
        if (m_vehicles[i].active)
        {
            activeVehicles++;
        }
        else if (availableSlot == -1)
        {
            availableSlot = i;
        }
    }

    // Don't spawn if we're at max capacity or no slot available
    if (activeVehicles >= MAX_VEHICLES || availableSlot == -1)
    {
        return;
    }

    Direction direction = getRandomDirection();

    // Check direction constraints
    if ((direction == Direction::LEFT && m_leftVehicleCount >= MAX_LEFT_VEHICLES) ||
        (direction == Direction::RIGHT && m_rightVehicleCount >= MAX_RIGHT_VEHICLES))
    {
        return;
    }

    // Create new vehicle
    Vehicle &newVehicle = m_vehicles[availableSlot];
    newVehicle.type = getRandomVehicleType();
    newVehicle.direction = direction;
    newVehicle.speed = MIN_SPEED + (static_cast<float>(random()) / RAND_MAX) * (MAX_SPEED - MIN_SPEED);

    // Set Y position based on a direction (simulate opposing traffic lanes)
    const int halfHeight = u8g2->height / 2;
    if (direction == Direction::RIGHT)
    {
        // Vehicles going LEFT use bottom half of screen
        newVehicle.y = halfHeight + 8 + (random() % (halfHeight - 24));
        m_rightVehicleCount++;
    }
    else // Direction::RIGHT
    {
        // Vehicles going RIGHT use top half of screen
        newVehicle.y = 8 + (random() % (halfHeight - 24));
        m_leftVehicleCount++;
    }

    // Set the starting X position based on the direction
    if (direction == Direction::LEFT)
    {
        // Vehicles going LEFT (from right to left) start from RIGHT side of screen
        newVehicle.x = u8g2->width + 16;
    }
    else // Direction::RIGHT
    {
        // Vehicles going RIGHT (from left to right) start from LEFT side of screen
        newVehicle.x = -32; // Account for the largest vehicle width
    }

    newVehicle.active = true;
}

ScreenSaver::VehicleType ScreenSaver::getRandomVehicleType()
{
    switch (random() % 5)
    {
    case 0:
        return VehicleType::CAR;
    case 1:
        return VehicleType::CONVERTABLE;
    case 2:
        return VehicleType::SUV;
    case 3:
        return VehicleType::LORRY;
    case 4:
        return VehicleType::TRUCK;
    default:
        return VehicleType::CAR;
    }
}

ScreenSaver::Direction ScreenSaver::getRandomDirection()
{
    // Simple 50/50 chance for each direction
    return (random() % 2 == 0) ? Direction::LEFT : Direction::RIGHT;
}

void ScreenSaver::render()
{
    // Clear screen with a black background
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);
    u8g2_SetDrawColor(u8g2, 1);

    // Calculate offsets
    const int roadOffset = (m_animationCounter / 100) % road_horizontal_width;
    
    // Draw all active vehicles with a scene offset
    for (const auto &vehicle : m_vehicles)
    {
        if (vehicle.active)
        {
            Vehicle offsetVehicle = vehicle;
            offsetVehicle.x += m_sceneOffsetX;
            offsetVehicle.y += m_sceneOffsetY;
            drawVehicle(offsetVehicle);
        }
    }

    // Draw road with offsets
    const int y = u8g2->height / 2 - road_horizontal_height / 2 + m_sceneOffsetY;
    for (int x = -road_horizontal_width + roadOffset + m_sceneOffsetX; x <= u8g2->width; x += road_horizontal_width)
    {
        drawTransparentBitmap(x, y, road_horizontal_width, road_horizontal_height, road_horizontal_bits);
    }
}

void ScreenSaver::drawVehicle(const Vehicle &vehicle) const
{
    int width, height;

    if (const unsigned char *bitmap = getVehicleBitmap(vehicle.type, vehicle.direction, width, height))
    {
        drawTransparentBitmap(vehicle.x, vehicle.y, width, height, bitmap);
        // u8g2_DrawXBM(u8g2, vehicle.x, vehicle.y, width, height, bitmap);
    }
}

void ScreenSaver::drawTransparentBitmap(const int x, const int y, const int width, const int height,
                                        const unsigned char *bitmap) const
{
    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            // Calculate byte and a bit of position in bitmap
            const int byteIndex = (py * ((width + 7) / 8)) + (px / 8);

            // Check if the pixel is set (white)
            if (const int bitIndex = px % 8; bitmap[byteIndex] & (1 << bitIndex))
            {
                // Only draw white pixels, skip black (transparent) pixels
                const int screenX = x + px;

                // Bounds checking
                if (const int screenY = y + py; screenX >= 0 && screenX < u8g2->width &&
                                                screenY >= 0 && screenY < u8g2->height)
                {
                    u8g2_DrawPixel(u8g2, screenX, screenY);
                }
            }
            // Black pixels are simply not drawn (transparent)
        }
    }
}

const unsigned char *ScreenSaver::getVehicleBitmap(const VehicleType type, const Direction direction, int &width,
                                                   int &height)
{
    switch (type)
    {
    case VehicleType::CAR:
        width = car_width;
        height = car_height;
        return (direction == Direction::LEFT) ? car_left_bits : car_right_bits;

    case VehicleType::CONVERTABLE:
        width = convertable_width;
        height = convertable_height;
        return (direction == Direction::LEFT) ? convertable_left_bits : convertable_right_bits;

    case VehicleType::SUV:
        width = suv_width;
        height = suv_height;
        return (direction == Direction::LEFT) ? suv_left_bits : suv_right_bits;

    case VehicleType::LORRY:
        width = lorry_width;
        height = lorry_height;
        return (direction == Direction::LEFT) ? lorry_left_bits : lorry_right_bits;

    case VehicleType::TRUCK:
        width = truck_width;
        height = truck_height;
        return (direction == Direction::LEFT) ? truck_left_bits : truck_right_bits;

    default:
        width = car_width;
        height = car_height;
        return car_left_bits;
    }
}

void ScreenSaver::onButtonClicked(ButtonType button)
{
    if (m_options && m_options->popScreen)
    {
        m_options->popScreen();
    }
}