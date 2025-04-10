#include "ui/UIWidget.h"

UIWidget::UIWidget(void *appstate): m_context(static_cast<AppContext *>(appstate)) {
}

UIWidget::~UIWidget() = default;

auto UIWidget::get_context() const -> AppContext* {
    return m_context;
}
