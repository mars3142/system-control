#pragma once

#include <functional>

#include "Common.h"
#include "MenuOptions.h"
#include "Widget.h"
#include "data/MenuItem.h"

class PSMenu : public Widget
{
public:
    explicit PSMenu(menu_options_t *options);
    ~PSMenu() override;

    void addText(uint8_t id, const std::string &text, const ButtonCallback &callback);
    void addSelection(uint8_t id, const std::string &text, std::string &value, const std::vector<std::string>& values,
                      const ButtonCallback &callback);
    void addNumber(uint8_t id, const std::string &text, std::string &value, const ButtonCallback &callback);
    void addToggle(uint8_t id, const std::string &text, bool selected, const ButtonCallback &callback);

private:
    void render() override;
    void onButtonClicked(uint8_t button) override;

    void onPressedDown();
    void onPressedUp();
    void onPressedLeft() const;
    void onPressedRight() const;
    void onPressedSelect() const;
    void onPressedBack() const;

    void drawScrollBar() const;
    void drawSelectionBox() const;

    void renderWidget(const MenuItem *item, const uint8_t *font, int x, int y) const;

    size_t m_selected_item = 0;
    std::vector<MenuItem> m_items;
    menu_options_t *m_options;
};