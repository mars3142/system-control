#include "ui/ClockScreenSaver.h"
#include "hal_esp32/PersistenceManager.h"
#include "simulator.h"
#include <cstring>
#include <ctime>

ClockScreenSaver::ClockScreenSaver(menu_options_t *options)
    : Widget(options->u8g2), m_options(options), m_moveTimer(0), m_posX(0), m_posY(0), m_velocityX(X_VELOCITY),
      m_velocityY(Y_VELOCITY), m_textWidth(0), m_textHeight(0)
{
    initPosition();
}

void ClockScreenSaver::initPosition()
{
    // Start in the center of the screen
    updateTextDimensions();
    m_posX = (u8g2->width - m_textWidth) / 2;
    m_posY = (u8g2->height - m_textHeight) / 2;

    // Set initial velocity
    m_velocityX = X_VELOCITY;
    m_velocityY = Y_VELOCITY;
}

void ClockScreenSaver::updateTextDimensions()
{
    char timeBuffer[32];
    getCurrentTimeString(timeBuffer, sizeof(timeBuffer));

    // Set font (use a large font for better visibility)
    u8g2_SetFont(u8g2, FONT);

    // Calculate text dimensions
    m_textWidth = u8g2_GetStrWidth(u8g2, timeBuffer);
    m_textHeight = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2);
}

void ClockScreenSaver::getCurrentTimeString(char *buffer, size_t bufferSize) const
{
    if (m_options && m_options->persistenceManager->GetValue("light_active", false) &&
        m_options->persistenceManager->GetValue("light_mode", 0) == 0)
    {
        char *simulated_time = get_time();
        if (simulated_time != nullptr)
        {
            strncpy(buffer, simulated_time, bufferSize);
            return;
        }
    }
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Format time as HH:MM:SS
    strftime(buffer, bufferSize, "%H:%M:%S", timeinfo);
}

void ClockScreenSaver::Update(const uint64_t dt)
{
    m_moveTimer += dt;

    // Move the clock position at regular intervals
    if (m_moveTimer > MOVE_INTERVAL)
    {
        m_moveTimer = 0;

        // Update text dimensions (in case time format changes)
        updateTextDimensions();

        // Update position
        m_posX += m_velocityX;
        m_posY += m_velocityY;

        // Check for collisions with screen boundaries
        checkBoundaryCollision();
    }
}

void ClockScreenSaver::checkBoundaryCollision()
{
    // Check horizontal boundaries
    if (m_posX <= TEXT_PADDING)
    {
        m_posX = TEXT_PADDING;
        m_velocityX = X_VELOCITY; // Bounce right
    }
    else if (m_posX + m_textWidth >= u8g2->width - TEXT_PADDING)
    {
        m_posX = u8g2->width - m_textWidth - TEXT_PADDING;
        m_velocityX = -X_VELOCITY; // Bounce left
    }

    // Check vertical boundaries
    if (m_posY <= TEXT_PADDING + m_textHeight)
    {
        m_posY = TEXT_PADDING + m_textHeight;
        m_velocityY = Y_VELOCITY; // Bounce down
    }
    else if (m_posY >= u8g2->height - TEXT_PADDING)
    {
        m_posY = u8g2->height - TEXT_PADDING;
        m_velocityY = -Y_VELOCITY; // Bounce up
    }
}

void ClockScreenSaver::Render()
{
    // Clear screen with a black background
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);
    u8g2_SetDrawColor(u8g2, 1);

    // Get current time
    char timeBuffer[32];
    getCurrentTimeString(timeBuffer, sizeof(timeBuffer));

    // Set font
    u8g2_SetFont(u8g2, FONT);

    // Draw the time at current position
    u8g2_DrawStr(u8g2, m_posX, m_posY, timeBuffer);
}

void ClockScreenSaver::OnButtonClicked(ButtonType button)
{
    // Exit screensaver on any button press
    if (m_options && m_options->popScreen)
    {
        m_options->popScreen();
    }
}
