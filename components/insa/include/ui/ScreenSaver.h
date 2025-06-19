#pragma once

#include "MenuOptions.h"
#include "common/Widget.h"
#include <cstdlib>
#include <vector>

class ScreenSaver final : public Widget
{
  public:
    explicit ScreenSaver(menu_options_t *options);

    void update(uint64_t dt) override;
    void render() override;
    void onButtonClicked(ButtonType button) override;

  private:
    struct Star
    {
        float x;
        float y;
        float z;
        float speed;
    };

    menu_options_t *m_options;
    uint64_t m_animationCounter;
    std::vector<Star> m_stars;

    static constexpr int NUM_STARS = 10;
    static constexpr float SPEED_MULTIPLIER = 0.02f;
    static constexpr float Z_NEAR = 0.1f;
    static constexpr float Z_FAR = 10.0f;

    void initStars();
    void resetStar(Star &star);
};
