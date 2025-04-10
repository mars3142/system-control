#pragma once

#include "model/Window.h"

class Matrix {
public:
    explicit Matrix(Window* window);

    [[nodiscard]] Window* window() const;

    void render() const;

private:
    void draw_colored_grid(int rows, int cols, float cellSize, float spacing) const;

    Window* m_window;
};
