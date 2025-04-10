#include "ui/widgets/D_Pad.h"

#include <ResourceManager.h>

D_Pad::D_Pad(
    void* appState,
    const float x,
    const float y,
    const float width,
    std::function<void(int)> callback)
    : UIWidget(appState)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_callback(std::move(callback)) {
}

void D_Pad::render() const {
    const auto dPad = ResourceManager::getInstance().get_texture(
        get_context()->renderer(), "assets/d-pad_normal.png");

    const auto dst = SDL_FRect(m_x, m_y, m_width, m_width);
    SDL_RenderTexture(get_context()->renderer(), dPad, nullptr, &dst);
}
