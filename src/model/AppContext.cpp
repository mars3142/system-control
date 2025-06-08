#include "model/AppContext.h"

#include "ui/Matrix.h"

auto AppContext::MainWindow() const -> SDL_Window *
{
    return m_window->window();
}

auto AppContext::MainRenderer() const -> SDL_Renderer *
{
    return m_window->renderer();
}

auto AppContext::MainSurface() const -> SDL_Surface *
{
    return SDL_GetWindowSurface(m_window->window());
}

void AppContext::SetMatrix(Matrix *matrix)
{
    m_matrix = matrix;
}

auto AppContext::LedMatrix() const -> Matrix *
{
    return m_matrix;
}

auto AppContext::LedMatrixWindow() const -> SDL_Window *
{
    if (m_matrix && m_matrix->window())
    {
        return m_matrix->window()->window();
    }
    return nullptr;
}

auto AppContext::LedMatrixRenderer() const -> SDL_Renderer *
{
    if (m_matrix && m_matrix->window())
    {
        return m_matrix->window()->renderer();
    }
    return nullptr;
}

void AppContext::Render() const
{
    if (m_matrix && m_matrix->window())
    {
        m_matrix->Render();
    }
}
