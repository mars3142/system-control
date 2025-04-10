#pragma once

#include "../model/AppContext.h"

class UIWidget {
public:
    explicit UIWidget(void *appstate);

    virtual ~UIWidget();

    virtual void render() const = 0;

    [[nodiscard]] AppContext *get_context() const;

private:
    AppContext *m_context{};
};
