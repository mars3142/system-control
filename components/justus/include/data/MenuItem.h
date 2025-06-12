#pragma once

#include <functional>
#include <string>

#include "common/Common.h"

class MenuItem
{
public:
    MenuItem(uint8_t id, uint8_t type, std::string text, ButtonCallback callback);
    MenuItem(uint8_t id, uint8_t type, std::string text, std::string value, ButtonCallback callback);
    MenuItem(uint8_t id, uint8_t type, std::string text, std::string value, std::vector<std::string> values,
             ButtonCallback callback);
    MenuItem(uint8_t id, uint8_t type, std::string text, bool selected, ButtonCallback callback);
    [[nodiscard]] uint8_t getId() const;
    [[nodiscard]] uint8_t getType() const;
    [[nodiscard]] const std::string &getText() const;
    [[nodiscard]] const std::string &getValue() const;
    void setValue(const std::string &value);
    void onButtonPressed(uint8_t id, ButtonType button) const;
    [[nodiscard]] bool hasCallback() const;

private:
    uint8_t m_id;
    uint8_t m_type;
    std::string m_text;
    std::string m_value;
    std::vector<std::string> m_values;
    ButtonCallback m_callback;
};