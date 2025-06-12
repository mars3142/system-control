#include "data/MenuItem.h"

MenuItem::MenuItem(const uint8_t id, const uint8_t type, std::string text, ButtonCallback callback)
    : m_id(id), m_type(type), m_text(std::move(text)), m_callback(std::move(callback))
{
}

MenuItem::MenuItem(const uint8_t id, const uint8_t type, std::string text, std::string value,
                   ButtonCallback callback)
    : m_id(id), m_type(type), m_text(std::move(text)), m_value(std::move(value)), m_callback(std::move(callback))
{
}

MenuItem::MenuItem(const uint8_t id, const uint8_t type, std::string text, std::string value,
                   std::vector<std::string> values,
                   ButtonCallback callback)
    : m_id(id), m_type(type), m_text(std::move(text)), m_value(std::move(value)), m_values(std::move(values)),
      m_callback(std::move(callback))
{
}

MenuItem::MenuItem(const uint8_t id, const uint8_t type, std::string text, const bool selected,
                   ButtonCallback callback)
    : m_id(id), m_type(type), m_text(std::move(text)), m_value(selected ? "true" : "false"),
      m_callback(std::move(callback))
{
}

uint8_t MenuItem::getId() const
{
    return m_id;
}

uint8_t MenuItem::getType() const
{
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

void MenuItem::onButtonPressed(const uint8_t id, const ButtonType button) const
{
    if (m_callback)
    {
        m_callback(id, button);
    }
}

bool MenuItem::hasCallback() const
{
    return (m_callback != nullptr);
}