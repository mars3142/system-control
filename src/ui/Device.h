#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "UIWidget.h"
#include "persistence.h"
#include "common/Common.h"
#include "common/Widget.h"
#include "model/AppContext.h"

class Device final : public UIWidget
{
public:
    explicit Device(void *appstate);

    void Render(uint64_t dt) const override;

    void HandleTap(const SDL_MouseButtonEvent *event) const;

    void ReleaseTap(const SDL_MouseButtonEvent *event) const;

    void OnButtonClicked(ButtonType button) const;

    [[nodiscard]] bool IsHit(int mouse_x, int mouse_y) const override;

    void OnTap(int mouse_x, int mouse_y) override;

    void ReleaseTap(int mouse_x, int mouse_y) override;

private:
    void DrawBackground() const;

    void DrawScreen(uint64_t dt) const;

    void RenderU8G2() const;

    void SetScreen(const std::shared_ptr<Widget> &screen);

    void PushScreen(const std::shared_ptr<Widget> &screen);

    void PopScreen();

    static void PushKey(SDL_Keycode key);

    std::vector<std::shared_ptr<UIWidget>> m_children{};
    std::shared_ptr<Widget> m_widget;
    std::vector<std::shared_ptr<Widget>> m_history;
    persistence_t m_persistence{};
};