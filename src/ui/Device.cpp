#include "ui/Device.h"

#include <hal/u8g2_hal_sdl.h>
#include <u8g2.h>

#include "MenuOptions.h"
#include "common/InactivityTracker.h"
#include "ui/ScreenSaver.h"
#include "ui/SplashScreen.h"
#include "ui/widgets/Button.h"
#include "ui/widgets/D_Pad.h"

u8g2_t u8g2;
menu_options_t options;
std::unique_ptr<InactivityTracker> m_inactivityTracker;

static void set_pixel_rgba(const SDL_Surface *surface, const int x, const int y, const uint32_t pixel_color)
{
    if (!surface || x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    {
        return;
    }
    const auto p = static_cast<uint8_t *>(surface->pixels) + y * surface->pitch + x * sizeof(uint32_t);
    *reinterpret_cast<uint32_t *>(p) = pixel_color;
}

Device::Device(void *appstate) : UIWidget(appstate)
{
    auto dpad_callback = [](const D_Pad::Direction direction) {
        SDL_Keycode key = 0;
        switch (direction)
        {
        case D_Pad::Direction::UP:
            key = SDLK_UP;
            break;
        case D_Pad::Direction::DOWN:
            key = SDLK_DOWN;
            break;
        case D_Pad::Direction::LEFT:
            key = SDLK_LEFT;
            break;
        case D_Pad::Direction::RIGHT:
            key = SDLK_RIGHT;
            break;
        case D_Pad::Direction::NONE: // Fallthrough oder keine Aktion
        default:
            break;
        }
        if (key != 0)
        {
            PushKey(key);
        }
    };

    m_children.push_back(std::make_shared<Button>(
        GetContext(), U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 3 * U8G2_SCREEN_PADDING + DPAD_WIDTH,
        U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - BUTTON_WIDTH, BUTTON_WIDTH,
        []() {
            PushKey(SDLK_RETURN);
        }));
    m_children.push_back(std::make_shared<Button>(
        GetContext(), U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 4 * U8G2_SCREEN_PADDING + DPAD_WIDTH + BUTTON_WIDTH,
        U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - 2 * BUTTON_WIDTH, BUTTON_WIDTH,
        []() {
            PushKey(SDLK_BACKSPACE);
        }));
    m_children.push_back(std::make_shared<D_Pad>(
        GetContext(), U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 2 * U8G2_SCREEN_PADDING,
        U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - DPAD_WIDTH, DPAD_WIDTH, dpad_callback));

    u8g2_Setup_sh1106_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sdl_hw_spi, u8x8_gpio_and_delay_sdl);
    u8x8_InitDisplay(u8g2_GetU8x8(&u8g2));

    options = {
        .u8g2 = &u8g2,
        .setScreen = [this](const std::shared_ptr<Widget> &screen) {
            this->SetScreen(screen);
        },
        .pushScreen = [this](const std::shared_ptr<Widget> &screen) {
            this->PushScreen(screen);
        },
        .popScreen = [this]() {
            this->PopScreen();
        },
    };
    m_widget = std::make_shared<SplashScreen>(&options);
    m_inactivityTracker = std::make_unique<InactivityTracker>(60000, []() {
        const auto screensaver = std::make_shared<ScreenSaver>(&options);
        options.pushScreen(screensaver);
    });
}

void Device::SetScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        m_widget = screen;
        m_history.clear();
        m_history.emplace_back(m_widget);
        m_widget->enter();
    }
}

void Device::PushScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        if (m_widget)
        {
            m_widget->pause();
        }
        m_widget = screen;
        m_history.emplace_back(m_widget);
        m_widget->enter();
    }
}

void Device::PopScreen()
{
    if (m_history.size() >= 2)
    {
        if (m_widget)
        {
            m_widget->exit();
        }
        m_history.pop_back();
        m_widget = m_history.back();
        m_widget->resume();
    }
}

void Device::PushKey(const SDL_Keycode key)
{
    SDL_Event ev;
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    SDL_PushEvent(&ev);
}

void Device::RenderU8G2() const
{
    SDL_Surface *u8g2_surface = nullptr;
    SDL_Texture *u8g2_texture = nullptr;

    u8g2_surface = SDL_CreateSurface(U8G2_SCREEN_WIDTH, U8G2_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888);

    if (!u8g2_surface)
    {
        SDL_Log("SDL_CreateSurfaceFrom Error: %s\n", SDL_GetError());
    }
    else
    {
        const auto color_black = SDL_MapSurfaceRGBA(u8g2_surface, 0, 0, 0, 255);
        const auto color_white = SDL_MapSurfaceRGBA(u8g2_surface, 255, 255, 255, 255);

        const auto u8g2_buf = u8g2_GetBufferPtr(&u8g2);
        for (auto y = 0; y < U8G2_SCREEN_HEIGHT; ++y)
        {
            for (auto x = 0; x < U8G2_SCREEN_WIDTH; ++x)
            {
                const auto page = y / 8;
                const auto bit_index = y % 8;
                const auto byte_ptr = u8g2_buf + page * U8G2_SCREEN_WIDTH + x;
                const auto pixel_is_set = (*byte_ptr >> bit_index) & 0x01;
                set_pixel_rgba(u8g2_surface, x, y, pixel_is_set ? color_white : color_black);
            }
        }

        u8g2_texture = SDL_CreateTextureFromSurface(GetContext()->MainRenderer(), u8g2_surface);
        if (!u8g2_texture)
        {
            SDL_Log("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        }
        if (!SDL_SetTextureScaleMode(u8g2_texture, SDL_SCALEMODE_NEAREST))
        {
            SDL_Log("SDL_SetTextureScaleMode Error: %s\n", SDL_GetError());
        }
        SDL_DestroySurface(u8g2_surface);
    }

    if (u8g2_texture)
    {
        SDL_FRect destRect;
        destRect.x = U8G2_SCREEN_PADDING;
        destRect.y = U8G2_SCREEN_PADDING;
        destRect.w = U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR;
        destRect.h = U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR;

        SDL_RenderTexture(GetContext()->MainRenderer(), u8g2_texture, nullptr, &destRect);
        SDL_DestroyTexture(u8g2_texture);
    }
}

void Device::DrawBackground() const
{
    int windowWidth = 0;
    int windowHeight = 0;

    if (!SDL_GetWindowSize(GetContext()->MainWindow(), &windowWidth, &windowHeight))
    {
        SDL_Log("SDL_GetWindowSize Error: %s\n", SDL_GetError());
    }
    const auto rect = SDL_FRect{0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight)};
    SDL_SetRenderDrawColor(GetContext()->MainRenderer(), 193, 46, 31, 255);
    SDL_RenderFillRect(GetContext()->MainRenderer(), &rect);
}

void Device::DrawScreen(const uint64_t dt) const
{
    u8g2_ClearBuffer(&u8g2);

    if (m_widget != nullptr)
    {
        m_widget->update(dt);
        m_widget->render();
    }

    RenderU8G2();
}

void Device::Render(const uint64_t dt) const
{
    DrawBackground();
    DrawScreen(dt);

    for (const auto &child : m_children)
    {
        child->Render(dt);
    }

    m_inactivityTracker->update(dt);
}

void Device::HandleTap(const SDL_MouseButtonEvent *event) const
{
    // SDL_Log("HandleTap: x=%f, y=%f, button=%d", event->x, event->y, event->button);

    for (const auto &child : m_children)
    {
        if (child->IsHit(static_cast<int>(event->x), static_cast<int>(event->y)))
        {
            child->OnTap(static_cast<int>(event->x), static_cast<int>(event->y));
            break;
        }
    }
}

void Device::ReleaseTap(const SDL_MouseButtonEvent *event) const
{
    // SDL_Log("ReleaseTap: x=%f, y=%f, button=%d", event->x, event->y, event->button);
    m_inactivityTracker->reset();

    for (const auto &child : m_children)
    {
        child->ReleaseTap(static_cast<int>(event->x), static_cast<int>(event->y));
    }
}

void Device::OnButtonClicked(const ButtonType button) const
{
    m_inactivityTracker->reset();

    if (m_widget != nullptr)
    {
        m_widget->onButtonClicked(button);
    }
}

bool Device::IsHit(int mouse_x, int mouse_y) const
{
    return false;
}

void Device::OnTap(int mouse_x, int mouse_y)
{
    ////
}

void Device::ReleaseTap(int mouse_x, int mouse_y)
{
    ///
}