#pragma once

#include "ui/UIWidget.h"

#include <functional>

#define BUTTON_WIDTH (35)

class Button final : public UIWidget
{
  public:
    Button(void *appState, float x, float y, float width, std::function<void()> callback);

    void Render() const override;

    [[nodiscard]] bool IsHit(int mouse_x, int mouse_y) const override;

    void OnTap(int mouse_x, int mouse_y) override;

    void ReleaseTap(int mouse_x, int mouse_y) override;

private:
    float m_x;
    float m_y;
    float m_width;
    std::function<void()> m_callback;
    bool m_isPressed = false;
};
