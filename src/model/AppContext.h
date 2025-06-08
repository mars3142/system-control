#pragma once

#include "SDL3_ttf/SDL_ttf.h"
#include "Window.h"

class Matrix;

class AppContext
{
  public:
    explicit AppContext(const Window *window) : m_window(window)
    {
        m_font_default = TTF_OpenFont("haxrcorp-4089.otf", 21);
        m_font_text = TTF_OpenFont("Helvetica-Bold.otf", 21);
    }

    ~AppContext()
    {
        TTF_CloseFont(m_font_default);
        TTF_CloseFont(m_font_text);
    }

    [[nodiscard]] auto MainWindow() const -> SDL_Window *;

    [[nodiscard]] auto MainRenderer() const -> SDL_Renderer *;

    [[nodiscard]] auto MainSurface() const -> SDL_Surface *;

    void SetMatrix(Matrix *matrix);

    [[nodiscard]] auto LedMatrix() const -> Matrix *;

    [[nodiscard]] auto LedMatrixWindow() const -> SDL_Window *;

    [[nodiscard]] auto LedMatrixRenderer() const -> SDL_Renderer *;

    void Render() const;

    TTF_Font *m_font_default = nullptr;

  private:
    const Window *m_window;
    Matrix *m_matrix = nullptr;
    TTF_Font *m_font_text = nullptr;
};
