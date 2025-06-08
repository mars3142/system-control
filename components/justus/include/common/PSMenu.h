#pragma once

#include <functional>

#include "MenuOptions.h"
#include "Widget.h"
#include "data/MenuItem.h"

typedef std::function<void(uint8_t)> MenuCallback;

class PSMenu : public Widget
{
  public:
    explicit PSMenu(menu_options_t *options);
    ~PSMenu() override;

    void render() override;
    void onButtonClicked(uint8_t button) override;

    void addText(const std::string &text, const MenuCallback &callback);
    void addSwitch(const std::string &text, std::string &value, const MenuCallback &callback);
    void addNumber(const std::string &text, std::string &value, const MenuCallback &callback);

  private:
    void onPressedDown();
    void onPressedUp();
    void onPressedLeft();
    void onPressedRight();
    void onPressedSelect() const;
    void onPressedBack() const;

    void drawScrollBar() const;
    void drawSelectionBox() const;

    void renderWidget(uint8_t type, const uint8_t *font, int x, int y, const char *text) const;

    size_t m_selected_item = 0;
    std::vector<MenuItem> m_items;
    menu_options_t *m_options;
};
