#include "model/Window.h"

auto Window::window() const -> SDL_Window *
{
    return m_window;
}

auto Window::renderer() const -> SDL_Renderer *
{
    return SDL_GetRenderer(m_window);
}
