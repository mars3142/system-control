#pragma once

#include "../UIWidget.h"

#include <functional>

#define BUTTON_WIDTH (35)

class Button final : public UIWidget {
public:
    Button(void* appState, float x, float y, float width, std::function<void()> callback);

    void render() const override;

private:
    float m_x;
    float m_y;
    float m_width;
    std::function<void()> m_callback;
};
