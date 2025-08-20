#pragma once

#include "model/AppContext.h"

class UIWidget
{
public:
    explicit UIWidget(void *appstate);

    virtual ~UIWidget();

    virtual void Render(uint64_t dt) const = 0;

    [[nodiscard]] virtual bool IsHit(int mouse_x, int mouse_y) const = 0;

    virtual void OnTap(int mouse_x, int mouse_y) = 0;

    virtual void ReleaseTap(int mouse_x, int mouse_y) = 0;

    [[nodiscard]] AppContext *GetContext() const;

private:
    AppContext *m_context{};
};
