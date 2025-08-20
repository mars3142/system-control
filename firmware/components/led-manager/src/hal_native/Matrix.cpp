#include "Matrix.h"

#include "SDL3/SDL.h"

Matrix::Matrix(uint32_t windowID, SDL_Renderer *renderer, const uint8_t cols, const uint8_t rows)
    : m_windowId(windowID), m_renderer(renderer), m_cols(cols), m_rows(rows)
{
}

SDL_Renderer *Matrix::renderer() const
{
    return m_renderer;
}

SDL_WindowID Matrix::windowId() const
{
    return m_windowId;
}

void Matrix::DrawColoredGrid() const
{
    int i = 0;
    for (int w = 0; w < m_cols; w++)
    {
        const auto phase = w % (2 * m_rows);

        for (int h_raw = 0; h_raw < m_rows; h_raw++)
        {
            int h;
            if (phase < m_rows)
            {
                h = h_raw;
            }
            else
            {
                h = m_rows - 1 - h_raw;
            }

            constexpr auto rectSize = cellSize - 2.0f * spacing;

            const auto x = static_cast<float>(w) * cellSize + spacing;
            const auto y = static_cast<float>(h) * cellSize + spacing;

            auto rect = SDL_FRect{x, y, rectSize, rectSize};

            i++;
            const auto red = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            const auto green = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            const auto blue = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            SDL_SetRenderDrawColor(m_renderer, red, green, blue, 255);
            SDL_RenderFillRect(m_renderer, &rect);
        }
    }
}

void Matrix::Render() const
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    DrawColoredGrid();

    SDL_RenderPresent(m_renderer);
}
