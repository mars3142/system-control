#pragma once

#include "functional"
#include "ui/UIWidget.h"

#define DPAD_WIDTH (105)


class D_Pad final : public UIWidget
{
public:
    enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };

    D_Pad(void *appState, float x, float y, float width, std::function<void(Direction)> callback);

    void Render() const override;

    [[nodiscard]] bool IsHit(int mouse_x, int mouse_y) const override;

    void OnTap(int mouse_x, int mouse_y) override;

    void ReleaseTap(int mouse_x, int mouse_y) override;

private:
    float m_x, m_y, m_width;
    std::function<void(Direction)> m_callback;

    [[nodiscard]] Direction GetDirectionFromTap(float local_x, float local_y) const;
};
