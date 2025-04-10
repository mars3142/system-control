#include "model/AppContext.h"

#include "ui/Matrix.h"

auto AppContext::window() const -> SDL_Window * {
    return m_window->window();
}

auto AppContext::renderer() const -> SDL_Renderer * {
    return m_window->renderer();
}

auto AppContext::surface() const -> SDL_Surface * {
    return SDL_GetWindowSurface(m_window->window());
}

void AppContext::setMatrix(Matrix *matrix) {
    m_matrix = matrix;
}

auto AppContext::matrix() const -> Matrix * {
    return m_matrix;
}

auto AppContext::matrix_window() const -> SDL_Window * {
    if (m_matrix && m_matrix->window()) {
        return m_matrix->window()->window();
    }
    return nullptr;
}

auto AppContext::matrix_renderer() const -> SDL_Renderer * {
    if (m_matrix && m_matrix->window()) {
        return m_matrix->window()->renderer();
    }
    return nullptr;
}

void AppContext::matrix_render() const {
    if (m_matrix && m_matrix->window()) {
        m_matrix->render();
    }
}
