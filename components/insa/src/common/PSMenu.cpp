#include "common/PSMenu.h"

#include "PushButton.h"
#include "common/ScrollBar.h"
#include "u8g2.h"

PSMenu::PSMenu(menu_options_t *options) : Widget(options->u8g2), m_options(options)
{
    m_options->onButtonClicked = [this](const uint8_t button) {
        onButtonClicked(button);
    };
}

PSMenu::~PSMenu()
{
    m_options->onButtonClicked = nullptr;
}

void PSMenu::render()
{
    if (m_selected_item < 0)
    {
        onPressedDown();
    }

    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);

    u8g2_SetDrawColor(u8g2, 1);

    drawScrollBar();
    drawSelectionBox();

    int x = 8; // sure?
    auto widget = m_items.at(m_selected_item);
    renderWidget(&widget, u8g2_font_helvB08_tr, x, u8g2->height / 2 + 3);

    if (m_selected_item > 0)
    {
        auto item = m_items.at(m_selected_item - 1);
        renderWidget(&item, u8g2_font_haxrcorp4089_tr, x, 14);
    }
    if (m_selected_item < m_items.size() - 1)
    {
        auto item = m_items.at(m_selected_item + 1);
        renderWidget(&item, u8g2_font_haxrcorp4089_tr, x, u8g2->height - 10);
    }
}

void PSMenu::renderWidget(const MenuItem *item, const uint8_t *font, const int x, const int y) const
{
    u8g2_SetFont(u8g2, font);
    u8g2_DrawStr(u8g2, x, y, item->getText().c_str());
    switch (item->getType())
    {
    case 1: // Selection
    {
        std::string value = "< ";
        value += item->getValue();
        value += " >";
        const u8g2_uint_t w = u8g2_GetStrWidth(u8g2, value.c_str());
        u8g2_DrawStr(u8g2, u8g2->width - w - 10, y, value.c_str());
        break;
    }

    case 3: // toggle
    {
        u8g2_DrawFrame(u8g2, u8g2->width - 24, y - 11, 14, 14);
        if (strcmp(item->getValue().c_str(), "true") == 0)
        {
            u8g2_DrawLine(u8g2, u8g2->width - 22, y - 9, u8g2->width - 13, y);
            u8g2_DrawLine(u8g2, u8g2->width - 22, y, u8g2->width - 13, y - 9);
        }
        break;
    }

    default:


    }
}

void PSMenu::onButtonClicked(const uint8_t button)
{
    switch (button)
    {
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

void PSMenu::onPressedDown()
{
    if (m_selected_item == m_items.size() - 1)
    {
        m_selected_item = 0;
    }
    else
    {
        m_selected_item++;
    }
}

void PSMenu::onPressedUp()
{
    if (m_selected_item == 0)
    {
        m_selected_item = m_items.size() - 1;
    }
    else
    {
        m_selected_item--;
    }
}

void PSMenu::onPressedLeft() const
{
    const auto item = m_items.at(m_selected_item);
    item.onButtonPressed(item.getId(), ButtonType::LEFT);
}

void PSMenu::onPressedRight() const
{
    const auto item = m_items.at(m_selected_item);
    item.onButtonPressed(item.getId(), ButtonType::RIGHT);
}

void PSMenu::onPressedSelect() const
{
    const auto item = m_items.at(m_selected_item);
    item.onButtonPressed(item.getId(), ButtonType::SELECT);
}

void PSMenu::onPressedBack() const
{
    if (m_options && m_options->popScreen)
    {
        m_options->popScreen();
    }
}

void PSMenu::addText(uint8_t id, const std::string &text, const ButtonCallback &callback)
{
    m_items.emplace_back(id, 0, text, callback);
}

void PSMenu::addSelection(uint8_t id, const std::string &text, std::string &value,
                          const std::vector<std::string> &values,
                          const ButtonCallback &callback)
{
    m_items.emplace_back(id, 1, text, value, values, callback);
}

void PSMenu::addNumber(uint8_t id, const std::string &text, std::string &value, const ButtonCallback &callback)
{
    m_items.emplace_back(id, 2, text, value, callback);
}

void PSMenu::addToggle(uint8_t id, const std::string &text, bool selected, const ButtonCallback &callback)
{
    m_items.emplace_back(id, 3, text, selected, callback);
}

void PSMenu::drawScrollBar() const
{
    ScrollBar scrollBar(m_options, u8g2->width - 3, 3, 1, u8g2->height - 6);
    scrollBar.refresh(m_selected_item, m_items.size());
    scrollBar.render();
}

void PSMenu::drawSelectionBox() const
{
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