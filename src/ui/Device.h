#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "UIWidget.h"
#include "model/AppContext.h"
#include "common/Widget.h"

class Device final : public UIWidget {
public:
    explicit Device(void* appstate);

    void render() const override;

    void hit_test(SDL_MouseMotionEvent* event) const;

    void onButtonClicked(uint8_t button) const;

private:
    void draw_background() const;

    void draw_screen() const;

    void draw_text() const;

    void render_u8g2() const;

    void setScreen(const std::shared_ptr<Widget>& screen);

    void pushScreen(const std::shared_ptr<Widget>& screen);

    void popScreen();

    static void pushKey(SDL_Keycode key);

    std::vector<std::shared_ptr<UIWidget>> m_children{};

    std::shared_ptr<Widget> widget;
    std::vector<std::shared_ptr<Widget>> history;
};
