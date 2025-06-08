#pragma once

#include "model/Window.h"

class Matrix
{
  public:
    explicit Matrix(Window *window);

    [[nodiscard]] Window *window() const;

    void Render() const;

  private:
    void DrawColoredGrid(int rows, int cols, float cellSize, float spacing) const;

    Window *m_window;
};
