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

MenuItem::MenuItem(const uint8_t id, const uint8_t type, std::string text, std::vector<std::string> values, int index,
                   ButtonCallback callback)
    : m_id(id), m_type(type), m_text(std::move(text)), m_values(std::move(values)), m_index(index),
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
    if (!m_values.empty() && m_index >= 0 && m_index < m_values.size())
    {
        return m_values.at(m_index);
    }
    return m_value;
}

void MenuItem::setValue(const std::string &value)
{
    m_value = value;
}

void MenuItem::onButtonPressed(const ButtonType button) const
{
    if (m_callback)
    {
        m_callback(*this, button);
    }
}

bool MenuItem::hasCallback() const
{
    return (m_callback != nullptr);
}

int MenuItem::getIndex() const
{
    return m_index;
}

std::vector<std::string> MenuItem::getValues() const
{
    return m_values;
}

size_t MenuItem::getItemCount() const
{
    return m_values.size();
}

MenuItem MenuItem::copyWith(const size_t index) const
{
    MenuItem copy = *this;
    if (index > std::numeric_limits<int>::max())
    {
        throw std::overflow_error("index is too large");
    }
    copy.m_index = static_cast<int>(index);
    return copy;
}