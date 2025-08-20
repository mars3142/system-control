#include "ui/UIWidget.h"

UIWidget::UIWidget(void *appstate) : m_context(static_cast<AppContext *>(appstate))
{
}

UIWidget::~UIWidget() = default;

auto UIWidget::GetContext() const -> AppContext *
{
    return m_context;
}
