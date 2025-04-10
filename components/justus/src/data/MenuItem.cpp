#include "data/MenuItem.h"

MenuItem::MenuItem(const uint8_t type, std::string text, std::function<void(uint8_t)> callback)
    : m_type(type)
    , m_text(std::move(text))
    , m_callback(std::move(callback)) {
}

MenuItem::MenuItem(
    const uint8_t type,
    std::string text,
    std::string value,
    std::function<void(uint8_t)> callback)
    : m_type(type)
    , m_text(std::move(text))
    , m_value(std::move(value))
    , m_callback(std::move(callback)) {
}

uint8_t MenuItem::getType() const {
    return m_type;
}

const std::string &MenuItem::getText() const
{
    return m_text;
}

const std::string &MenuItem::getValue() const
{
    return m_value;
}

void MenuItem::setValue(const std::string &value)
{
    m_value = value;
}

void MenuItem::callback(const uint8_t id) const
{
    if (m_callback)
    {
        m_callback(id);
    }
    else
    {
        ///
    }
}

bool MenuItem::hasCallback() const
{
    return (m_callback != nullptr);
}
