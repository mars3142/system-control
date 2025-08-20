#pragma once

#include "SDL3/SDL_render.h"

#include <cstdint>

class Matrix
{
  public:
    explicit Matrix(SDL_WindowID windowId, SDL_Renderer *renderer, uint8_t cols, uint8_t rows);

    [[nodiscard]] SDL_Renderer *renderer() const;

    void Render() const;

    [[nodiscard]] SDL_WindowID windowId() const;

  private:
    void DrawColoredGrid() const;

    SDL_WindowID m_windowId;
    SDL_Renderer *m_renderer;

    uint8_t m_cols;
    uint8_t m_rows;

    static constexpr float cellSize = 50.0f;
    static constexpr float spacing = 1.0f;
};
