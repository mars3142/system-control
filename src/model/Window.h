#pragma once

#include "SDL3/SDL.h"

class Window {
public:
    explicit Window(SDL_Window* window)
        : m_window(window) {
    }

    [[nodiscard]] auto window() const -> SDL_Window*;

    [[nodiscard]] auto renderer() const -> SDL_Renderer*;

private:
    SDL_Window* m_window = nullptr;
};
