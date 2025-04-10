#include "ui/widgets/Button.h"

#include <functional>
#include <utility>
#include <SDL3_image/SDL_image.h>
#include "ResourceManager.h"

auto pressed = false;

Button::Button(
    void* appState,
    const float x,
    const float y,
    const float width,
    std::function<void()> callback)
    : UIWidget(appState)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_callback(std::move(callback)) {
}

void Button::render() const {
    const auto button = ResourceManager::getInstance().get_texture(get_context()->renderer(), "assets/button_normal.png");
    const auto overlay =ResourceManager::getInstance().get_texture(get_context()->renderer(), "assets/button_pressed_overlay.png");

    const auto dst = SDL_FRect(m_x, m_y, m_width, m_width);
    SDL_RenderTexture(get_context()->renderer(), button, nullptr, &dst);
    if(pressed) {
        SDL_RenderTexture(get_context()->renderer(), overlay, nullptr, &dst);
    }
}
