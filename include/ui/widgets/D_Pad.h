#pragma once

#include "functional"

#include "ui/UIWidget.h"

#define DPAD_WIDTH (105)

class D_Pad final : public UIWidget {
public:
    D_Pad(void* appState, float x, float y, float width, std::function<void(int)> callback);

    void render() const override;

private:
    float m_x;
    float m_y;
    float m_width;
    std::function<void(int)> m_callback;
};
