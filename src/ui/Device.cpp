#include "ui/Device.h"

#include "SDL3_ttf/SDL_ttf.h"
#include "ui/widgets/Button.h"
#include "ui/widgets/D_Pad.h"

#include <u8g2.h>
#include <hal/u8g2_hal_sdl.h>

#include "ui/SplashScreen.h"
#include "MenuOptions.h"

u8g2_t u8g2;
menu_options_t options;

static void set_pixel_rgba(
    const SDL_Surface* surface,
    const int x,
    const int y,
    const uint32_t pixel_color) {
    if(!surface || x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        return;
    }
    const auto p =
        static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * sizeof(uint32_t);
    *reinterpret_cast<uint32_t*>(p) = pixel_color;
}

Device::Device(void* appstate)
    : UIWidget(appstate) {
    m_children.push_back(
        std::make_shared<Button>(
            get_context(),
            U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 3 * U8G2_SCREEN_PADDING + DPAD_WIDTH,
            U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - BUTTON_WIDTH,
            BUTTON_WIDTH,
            []() { pushKey(SDLK_RETURN); }));
    m_children.push_back(
        std::make_shared<Button>(
            get_context(),
            U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 4 * U8G2_SCREEN_PADDING + DPAD_WIDTH +
                BUTTON_WIDTH,
            U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - 2 * BUTTON_WIDTH,
            BUTTON_WIDTH,
            []() { pushKey(SDLK_BACKSPACE); }));
    m_children.push_back(
        std::make_shared<D_Pad>(
            get_context(),
            U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 2 * U8G2_SCREEN_PADDING,
            U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + U8G2_SCREEN_PADDING - DPAD_WIDTH,
            DPAD_WIDTH,
            [](int d) {}));

    u8g2_Setup_sh1106_128x64_noname_f(
        &u8g2, U8G2_R0, u8x8_byte_sdl_hw_spi, u8x8_gpio_and_delay_sdl);
    u8x8_InitDisplay(u8g2_GetU8x8(&u8g2));

    options = {
        .u8g2 = &u8g2,
        .setScreen = [this](const std::shared_ptr<Widget>& screen) { this->setScreen(screen); },
        .pushScreen = [this](const std::shared_ptr<Widget>& screen) { this->pushScreen(screen); },
        .popScreen = [this]() { this->popScreen(); },
    };
    widget = std::make_shared<SplashScreen>(&options);
}

void Device::setScreen(const std::shared_ptr<Widget>& screen) {
    if(screen != nullptr) {
        widget = screen;
        history.clear();
        history.emplace_back(widget);
    }
}

void Device::pushScreen(const std::shared_ptr<Widget>& screen) {
    if(screen != nullptr) {
        widget = screen;
        history.emplace_back(widget);
    }
}

void Device::popScreen() {
    if(history.size() >= 2) {
        history.pop_back();
        widget = history.back();
    }
}

void Device::pushKey(const SDL_Keycode key) {
    SDL_Event ev;
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    SDL_PushEvent(&ev);
}

void Device::render_u8g2() const {
    SDL_Surface* u8g2_surface = nullptr;
    SDL_Texture* u8g2_texture = nullptr;

    u8g2_surface =
        SDL_CreateSurface(U8G2_SCREEN_WIDTH, U8G2_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888);

    if(!u8g2_surface) {
        SDL_Log("SDL_CreateSurfaceFrom Error: %s\n", SDL_GetError());
    } else {
        const auto color_black = SDL_MapSurfaceRGBA(u8g2_surface, 0, 0, 0, 255);
        const auto color_white = SDL_MapSurfaceRGBA(u8g2_surface, 255, 255, 255, 255);

        if(!SDL_LockSurface(u8g2_surface)) {
            SDL_Log("SDL_LockSurface Error: %s\n", SDL_GetError());
        } else {
            const auto u8g2_buf = u8g2_GetBufferPtr(&u8g2);
            for(auto y = 0; y < U8G2_SCREEN_HEIGHT; ++y) {
                for(auto x = 0; x < U8G2_SCREEN_WIDTH; ++x) {
                    const auto page = y / 8;
                    const auto bit_index = y % 8;
                    const auto byte_ptr = u8g2_buf + page * U8G2_SCREEN_WIDTH + x;
                    const auto pixel_is_set = (*byte_ptr >> bit_index) & 0x01;
                    set_pixel_rgba(u8g2_surface, x, y, pixel_is_set ? color_white : color_black);
                }
            }
            SDL_UnlockSurface(u8g2_surface);
        }

        u8g2_texture = SDL_CreateTextureFromSurface(get_context()->renderer(), u8g2_surface);
        if(!u8g2_texture) {
            SDL_Log("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        }
        if(!SDL_SetTextureScaleMode(u8g2_texture, SDL_SCALEMODE_NEAREST)) {
            SDL_Log("SDL_SetTextureScaleMode Error: %s\n", SDL_GetError());
        }
        SDL_DestroySurface(u8g2_surface);
    }

    if(u8g2_texture) {
        SDL_FRect destRect;
        destRect.x = U8G2_SCREEN_PADDING;
        destRect.y = U8G2_SCREEN_PADDING;
        destRect.w = U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR;
        destRect.h = U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR;

        SDL_RenderTexture(get_context()->renderer(), u8g2_texture, nullptr, &destRect);
    }
}

void Device::draw_background() const {
    int windowWidth = 0;
    int windowHeight = 0;

    if(!SDL_GetWindowSize(get_context()->window(), &windowWidth, &windowHeight)) {
        SDL_Log("SDL_GetWindowSize Error: %s\n", SDL_GetError());
    }
    const auto rect =
        SDL_FRect{0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight)};
    SDL_SetRenderDrawColor(get_context()->renderer(), 193, 46, 31, 255);
    SDL_RenderFillRect(get_context()->renderer(), &rect);
}

void Device::draw_screen() const {
    u8g2_ClearBuffer(&u8g2);

    if(widget != nullptr) {
        widget->update(SDL_GetTicks());
        widget->render();
    }

    render_u8g2();
}

void Device::draw_text() const {
    constexpr auto color = SDL_Color({0, 255, 200});
    const auto surface =
        TTF_RenderText_Blended(get_context()->m_font_default, "HelloWorld SDL3 TTF", 0, color);
    const auto texture = SDL_CreateTextureFromSurface(get_context()->renderer(), surface);
    SDL_DestroySurface(surface);
    const SDL_FRect dstRect{
        500, 100, static_cast<float>(texture->w), static_cast<float>(texture->h)};
    SDL_RenderTexture(get_context()->renderer(), texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture); // ? Is this safe to do here ?
}

void Device::render() const {
    draw_background();
    draw_screen();
    // draw_text();

    for(const auto& child : m_children) {
        child->render();
    }
}

void Device::hit_test(SDL_MouseMotionEvent* event) const {
    SDL_Log("x: %f", event->x);
}

void Device::onButtonClicked(const uint8_t button) const {
    if(widget != nullptr) {
        widget->onButtonClicked(button);
    }
}
