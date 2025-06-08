#include "ui/Matrix.h"

Matrix::Matrix(Window *window) : m_window(window)
{
}

Window *Matrix::window() const
{
    return m_window;
}

void Matrix::DrawColoredGrid(const int rows, const int cols, const float cellSize, const float spacing) const
{
    int i = 0;
    for (int w = 0; w < cols; w++)
    {
        const auto phase = w % (2 * rows);

        for (int h_raw = 0; h_raw < rows; h_raw++)
        {
            int h;
            if (phase < rows)
            {
                h = h_raw;
            }
            else
            {
                h = rows - 1 - h_raw;
            }

            const auto rectSize = cellSize - 2.0f * spacing;

            const auto x = static_cast<float>(w) * cellSize + spacing;
            const auto y = static_cast<float>(h) * cellSize + spacing;

            auto rect = SDL_FRect{x, y, rectSize, rectSize};

            i++;
            const auto red = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            const auto green = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            const auto blue = static_cast<Uint8>(static_cast<float>(i) * 255.0f);
            SDL_SetRenderDrawColor(m_window->renderer(), red, green, blue, 255);
            SDL_RenderFillRect(m_window->renderer(), &rect);
        }
    }
}

void Matrix::Render() const
{
    SDL_SetRenderDrawColor(m_window->renderer(), 0, 0, 0, 255);
    SDL_RenderClear(m_window->renderer());

    DrawColoredGrid(8, 32, 50.0f, 1.0f);

    SDL_RenderPresent(m_window->renderer());
}
