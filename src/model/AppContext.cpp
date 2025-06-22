#include "model/AppContext.h"

#include "Matrix.h"

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

auto AppContext::LedMatrixId() const -> SDL_WindowID
{
    if (m_matrix)
    {
        return m_matrix->windowId();
    }
    return 0;
}

auto AppContext::LedMatrixRenderer() const -> SDL_Renderer *
{
    if (m_matrix && m_matrix->renderer())
    {
        return m_matrix->renderer();
    }
    return nullptr;
}

void AppContext::Render() const
{
    if (m_matrix && m_matrix->renderer())
    {
        m_matrix->Render();
    }
}
