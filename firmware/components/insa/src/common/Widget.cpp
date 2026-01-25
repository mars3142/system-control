#include "common/Widget.h"
#include <algorithm>

std::vector<Widget *> Widget::s_instances;

Widget::Widget(u8g2_t *u8g2) : u8g2(u8g2)
{
    s_instances.push_back(this);
    if (s_instances.size() == 1)
    {
        message_manager_register_listener(globalMessageCallback);
    }
}

Widget::~Widget()
{
    s_instances.erase(std::remove(s_instances.begin(), s_instances.end(), this), s_instances.end());
}

void Widget::onEnter()
{
}

void Widget::onPause()
{
}

void Widget::onResume()
{
}

void Widget::onExit()
{
}

void Widget::Update(uint64_t dt)
{
}

void Widget::Render()
{
}

void Widget::OnButtonClicked(ButtonType button)
{
}

const char *Widget::getName() const
{
    return "Widget";
}

void Widget::onMessageReceived(const message_t *msg)
{
}

void Widget::globalMessageCallback(const message_t *msg)
{
    for (auto *w : s_instances)
    {
        w->onMessageReceived(msg);
    }
}
