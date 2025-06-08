#pragma once

#include <functional>
#include <memory>

#include "common/Widget.h"
#include "u8g2.h"

class Widget;

typedef struct
{
    u8g2_t *u8g2;

    std::function<void(std::shared_ptr<Widget>)> setScreen;
    std::function<void(std::shared_ptr<Widget>)> pushScreen;
    std::function<void()> popScreen;

    std::function<void(uint8_t button)> onButtonClicked;
} menu_options_t;
