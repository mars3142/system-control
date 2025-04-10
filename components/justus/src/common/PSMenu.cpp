#include "common/PSMenu.h"

#include "u8g2.h"
#include "common/ScrollBar.h"
#include "PushButton.h"

PSMenu::PSMenu(menu_options_t* options)
    : Widget(options->u8g2)
    , m_options(options) {
    m_options->onButtonClicked = [this](const uint8_t button) { onButtonClicked(button); };
}

PSMenu::~PSMenu() {
    m_options->onButtonClicked = nullptr;
}

void PSMenu::render() {
    if(m_selected_item < 0) {
        onPressedDown();
    }

    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);

    u8g2_SetDrawColor(u8g2, 1);

    drawScrollBar();
    drawSelectionBox();

    int x = 8; // sure?
    auto widget = m_items.at(m_selected_item);
    renderWidget(
        widget.getType(), u8g2_font_helvB08_tr, x, u8g2->height / 2 + 3, widget.getText().c_str());

    if(m_selected_item > 0) {
        auto item = m_items.at(m_selected_item - 1);
        renderWidget(item.getType(), u8g2_font_haxrcorp4089_tr, x, 14, item.getText().c_str());
    }
    if(m_selected_item < m_items.size() - 1) {
        auto item = m_items.at(m_selected_item + 1);
        renderWidget(
            item.getType(),
            u8g2_font_haxrcorp4089_tr,
            x,
            u8g2->height - 10,
            item.getText().c_str());
    }
}

void PSMenu::renderWidget(
    const uint8_t type,
    const uint8_t* font,
    const int x,
    const int y,
    const char* text) const {
    switch(type) {
    case 0: // text
        u8g2_SetFont(u8g2, font);
        u8g2_DrawStr(u8g2, x, y, text);
        break;

    default:
        break;
    }
}

void PSMenu::onButtonClicked(uint8_t button) {
    switch(button) {
    case BUTTON_UP:
        onPressedUp();
        break;

    case BUTTON_DOWN:
        onPressedDown();
        break;

    case BUTTON_LEFT:
        onPressedLeft();
        break;

    case BUTTON_RIGHT:
        onPressedRight();
        break;

    case BUTTON_SELECT:
        onPressedSelect();
        break;

    case BUTTON_BACK:
        onPressedBack();
        break;

    default:
        break;
    }
}

void PSMenu::onPressedDown() {
    if(m_selected_item == m_items.size() - 1) {
        m_selected_item = 0;
    } else {
        m_selected_item++;
    }
}

void PSMenu::onPressedUp() {
    if(m_selected_item == 0) {
        m_selected_item = m_items.size() - 1;
    } else {
        m_selected_item--;
    }
}

void PSMenu::onPressedLeft() {
    //
}

void PSMenu::onPressedRight() {
    ///
}

void PSMenu::onPressedSelect() const {
    m_items.at(m_selected_item).callback(m_selected_item);
}

void PSMenu::onPressedBack() const {
    if(m_options && m_options->popScreen) {
        m_options->popScreen();
    }
}

void PSMenu::addText(const std::string& text, const MenuCallback& callback) {
    m_items.emplace_back(0, text, callback);
}

void PSMenu::addSwitch(const std::string& text, std::string& value, const MenuCallback& callback) {
    m_items.emplace_back(1, text, value, callback);
}

void PSMenu::addNumber(const std::string& text, std::string& value, const MenuCallback& callback) {
    m_items.emplace_back(2, text, value, callback);
}

void PSMenu::drawScrollBar() const {
    ScrollBar scrollBar(m_options, u8g2->width - 3, 3, 1, u8g2->height - 6);
    scrollBar.refresh(m_selected_item, m_items.size());
    scrollBar.render();
}

void PSMenu::drawSelectionBox() const {
    const auto displayHeight = u8g2->height;
    const auto displayWidth = u8g2->width;
    constexpr auto rightPadding = 8;
    const auto boxHeight = displayHeight / 3;
    const auto y = boxHeight * 2 - 2;
    const auto x = displayWidth - rightPadding;

    u8g2_DrawRFrame(u8g2, 2, boxHeight, displayWidth - rightPadding, boxHeight, 3);
    u8g2_DrawLine(u8g2, 4, y, displayWidth - rightPadding, y);
    u8g2_DrawLine(u8g2, x, y - boxHeight + 3, x, y - 1);
}
