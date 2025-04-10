#pragma once

#include <cstdint>
#include <functional>
#include <string>

class MenuItem {
public:
    MenuItem(uint8_t type, std::string text, std::function<void(uint8_t)> callback);
    MenuItem(
        uint8_t type,
        std::string  text,
        std::string  value,
        std::function<void(uint8_t)> callback);
    [[nodiscard]] uint8_t getType() const;
    [[nodiscard]] const std::string &getText() const;
    [[nodiscard]] const std::string &getValue() const;
    void setValue(const std::string &value);
    void callback(uint8_t id) const;
    [[nodiscard]] bool hasCallback() const;

private:
    uint8_t m_type;
    std::string m_text;
    std::string m_value;
    std::function<void(uint8_t)> m_callback;
};
