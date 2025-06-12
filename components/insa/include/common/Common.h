#pragma once

#include <functional>

enum class ButtonType { NONE, UP, DOWN, LEFT, RIGHT, SELECT, BACK };

typedef std::function<void(uint8_t, ButtonType)> ButtonCallback;
