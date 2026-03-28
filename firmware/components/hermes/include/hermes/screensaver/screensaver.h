#pragma once

#include <cstdint>

class Screensaver
{
  public:
    virtual ~Screensaver() = default;

    virtual void init() = 0;
    virtual void draw(uint64_t dt) = 0;
};
