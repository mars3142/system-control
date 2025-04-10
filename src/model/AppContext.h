#pragma once

#include "SDL3_ttf/SDL_ttf.h"
#include "Window.h"

class Matrix;

class AppContext {
public:
    explicit AppContext(const Window* window)
        : m_window(window) {
        m_font_default = TTF_OpenFont("assets/haxrcorp-4089.otf", 21);
        m_font_text = TTF_OpenFont("assets/Helvetica-Bold.otf", 21);
    }

    ~AppContext() {
        TTF_CloseFont(m_font_default);
        TTF_CloseFont(m_font_text);
    }

    [[nodiscard]] auto window() const -> SDL_Window*;

    [[nodiscard]] auto renderer() const -> SDL_Renderer*;

    [[nodiscard]] auto surface() const -> SDL_Surface*;

    void setMatrix(Matrix* matrix);

    [[nodiscard]] auto matrix() const -> Matrix*;

    [[nodiscard]] auto matrix_window() const -> SDL_Window*;

    [[nodiscard]] auto matrix_renderer() const -> SDL_Renderer*;

    void matrix_render() const;

    TTF_Font* m_font_default = nullptr;

private:
    const Window* m_window;
    Matrix* m_matrix = nullptr;
    TTF_Font* m_font_text = nullptr;
};
