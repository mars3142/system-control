#include "common/Menu.h"

#include "common/ScrollBar.h"
#include "u8g2.h"

// Menu item type constants for better readability
namespace MenuItemTypes
{
constexpr uint8_t TEXT = 0;
constexpr uint8_t SELECTION = 1;
constexpr uint8_t TOGGLE = 2;
constexpr uint8_t TEXT_COUNTER = 3;
} // namespace MenuItemTypes

// UI layout constants
namespace UIConstants
{
constexpr int LEFT_MARGIN = 8;
constexpr int RIGHT_PADDING = 8;
constexpr int SCROLLBAR_WIDTH = 3;
constexpr int FRAME_BOX_SIZE = 14;
constexpr int FRAME_OFFSET = 11;
constexpr int SELECTION_MARGIN = 10;
constexpr int CORNER_RADIUS = 3;
constexpr int LINE_SPACING = 14;
constexpr int BOTTOM_OFFSET = 10;
} // namespace UIConstants

Menu::Menu(menu_options_t *options) : Widget(options->u8g2), m_options(options)
{
    // Set up button callback using lambda to forward to member function
    m_options->onButtonClicked = [this](const ButtonType button) { onButtonClicked(button); };
}

Menu::~Menu()
{
    // Clean up callback to prevent dangling pointer
    if (m_options)
    {
        m_options->onButtonClicked = nullptr;
    }
}

MenuItem Menu::getItem(const int index)
{
    return m_items.at(index);
}

size_t Menu::getItemCount() const
{
    return m_items.size();
}

void Menu::setItemSize(const size_t size)
{
    if ((m_items.size() - 1) < size)
    {
        for (size_t i = m_items.size() - 1; i < size; i++)
        {
            auto caption = std::string("Bereich ") + std::to_string(i + 1);
            auto index = 0;
            if (m_options && m_options->persistenceManager)
            {
                constexpr int key_length = 20;
                char key[key_length] = "";
                snprintf(key, key_length, "section_%zu", i + 1);
                index = m_options->persistenceManager->GetValue(key, index);
            }
            addSelection(i + 1, caption, m_items.at(0).getValues(), index);
        }
    }
    else
    {
        m_items.erase(m_items.begin() + static_cast<int>(size + 1), m_items.end());
    }
}

void Menu::toggle(const MenuItem &menuItem)
{
    const auto item =
        menuItem.copyWith(menuItem.getValue() == std::to_string(true) ? std::to_string(false) : std::to_string(true));
    replaceItem(menuItem.getId(), item);
}

MenuItem Menu::switchValue(const MenuItem &menuItem, ButtonType button)
{
    MenuItem result = menuItem;
    switch (button)
    {
    case ButtonType::LEFT:
        if (menuItem.getIndex() > 0)
        {
            const auto item = menuItem.copyWith(menuItem.getIndex() - 1);
            result = replaceItem(menuItem.getId(), item);
        }
        else
        {
            const auto item = menuItem.copyWith(menuItem.getItemCount() - 1);
            result = replaceItem(menuItem.getId(), item);
        }
        break;

    case ButtonType::RIGHT:
        if (menuItem.getIndex() < menuItem.getItemCount() - 1)
        {
            const auto item = menuItem.copyWith(menuItem.getIndex() + 1);
            result = replaceItem(menuItem.getId(), item);
        }
        else
        {
            const auto item = menuItem.copyWith(0);
            result = replaceItem(menuItem.getId(), item);
        }
        break;

    default:
        break;
    }

    return result;
}

MenuItem Menu::replaceItem(const int index, const MenuItem &item)
{
    m_items.at(index) = item;
    return item;
}

void Menu::render()
{
    // Initialize selection if not set
    if (m_selected_item >= m_items.size() && !m_items.empty())
    {
        m_selected_item = 0;
    }

    // Early return if no items to render
    if (m_items.empty())
    {
        return;
    }

    // Clear screen with black background
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, u8g2->width, u8g2->height);

    // Set white foreground color for drawing
    u8g2_SetDrawColor(u8g2, 1);

    // Draw UI components
    drawScrollBar();
    drawSelectionBox();

    // Calculate center position for main item
    const int centerY = u8g2->height / 2 + 3;

    // Render the currently selected item (main/center item)
    const auto &selectedItem = m_items[m_selected_item];
    renderWidget(&selectedItem, u8g2_font_helvB08_tr, UIConstants::LEFT_MARGIN, centerY);

    // Render previous item (above) if available
    if (m_selected_item > 0)
    {
        const auto &prevItem = m_items[m_selected_item - 1];
        renderWidget(&prevItem, u8g2_font_haxrcorp4089_tr, UIConstants::LEFT_MARGIN, UIConstants::LINE_SPACING);
    }

    // Render next item (below) if available
    if (m_selected_item < m_items.size() - 1)
    {
        const auto &nextItem = m_items[m_selected_item + 1];
        renderWidget(&nextItem, u8g2_font_haxrcorp4089_tr, UIConstants::LEFT_MARGIN,
                     u8g2->height - UIConstants::BOTTOM_OFFSET);
    }
}

void Menu::renderWidget(const MenuItem *item, const uint8_t *font, const int x, const int y) const
{
    // Set font and draw main text
    u8g2_SetFont(u8g2, font);
    u8g2_DrawStr(u8g2, x, y, item->getText().c_str());

    // Render type-specific elements
    switch (item->getType())
    {
    case MenuItemTypes::TEXT: {
        const std::string formattedValue = ">";
        const u8g2_uint_t textWidth = u8g2_GetStrWidth(u8g2, formattedValue.c_str());
        u8g2_DrawStr(u8g2, u8g2->width - textWidth - UIConstants::SELECTION_MARGIN, y, formattedValue.c_str());
        break;
    }

    case MenuItemTypes::TEXT_COUNTER: {
        const std::string formattedValue = "(" + item->getValue() + ") >";
        const u8g2_uint_t textWidth = u8g2_GetStrWidth(u8g2, formattedValue.c_str());
        u8g2_DrawStr(u8g2, u8g2->width - textWidth - UIConstants::SELECTION_MARGIN, y, formattedValue.c_str());
        break;
    }

    case MenuItemTypes::SELECTION: {
        // Format selection value with angle brackets
        const std::string formattedValue = "< " + item->getValue() + " >";
        const u8g2_uint_t textWidth = u8g2_GetStrWidth(u8g2, formattedValue.c_str());
        u8g2_DrawStr(u8g2, u8g2->width - textWidth - UIConstants::SELECTION_MARGIN, y, formattedValue.c_str());
        break;
    }

    case MenuItemTypes::TOGGLE: {
        // Draw checkbox frame
        const int frameX = u8g2->width - UIConstants::FRAME_BOX_SIZE - UIConstants::SELECTION_MARGIN;
        const int frameY = y - UIConstants::FRAME_OFFSET;
        u8g2_DrawFrame(u8g2, frameX, frameY, UIConstants::FRAME_BOX_SIZE, UIConstants::FRAME_BOX_SIZE);

        // Draw checkmark (X) if toggle is true
        if (item->getValue() == std::to_string(true))
        {
            const int checkX1 = frameX + 2;
            const int checkY1 = frameY + 2;
            const int checkX2 = frameX + UIConstants::FRAME_BOX_SIZE - 3;
            const int checkY2 = frameY + UIConstants::FRAME_BOX_SIZE - 3;

            // Draw X pattern for checked state
            u8g2_DrawLine(u8g2, checkX1, checkY1, checkX2, checkY2);
            u8g2_DrawLine(u8g2, checkX1, checkY2, checkX2, checkY1);
        }
        break;
    }

    default:
        // No additional rendering needed for text and number types
        break;
    }
}

void Menu::onButtonClicked(const ButtonType button)
{
    // Map button input to navigation functions
    switch (button)
    {
    case ButtonType::UP:
        onPressedUp();
        break;

    case ButtonType::DOWN:
        onPressedDown();
        break;

    case ButtonType::LEFT:
        onPressedLeft();
        break;

    case ButtonType::RIGHT:
        onPressedRight();
        break;

    case ButtonType::SELECT:
        onPressedSelect();
        break;

    case ButtonType::BACK:
        onPressedBack();
        break;

    default:
        // Ignore unknown button inputs
        break;
    }
}

void Menu::onPressedDown()
{
    if (m_items.empty())
        return;

    // Wrap around to first item when at the end
    m_selected_item = (m_selected_item + 1) % m_items.size();
}

void Menu::onPressedUp()
{
    if (m_items.empty())
        return;

    // Wrap around to last item when at the beginning
    m_selected_item = (m_selected_item == 0) ? m_items.size() - 1 : m_selected_item - 1;
}

void Menu::onPressedLeft() const
{
    if (m_selected_item < m_items.size())
    {
        const auto &item = m_items.at(m_selected_item);
        item.onButtonPressed(ButtonType::LEFT);
    }
}

void Menu::onPressedRight() const
{
    if (m_selected_item < m_items.size())
    {
        const auto &item = m_items.at(m_selected_item);
        item.onButtonPressed(ButtonType::RIGHT);
    }
}

void Menu::onPressedSelect() const
{
    if (m_selected_item < m_items.size())
    {
        const auto &item = m_items.at(m_selected_item);
        item.onButtonPressed(ButtonType::SELECT);
    }
}

void Menu::onPressedBack() const
{
    // Navigate back to previous screen if callback is available
    if (m_options && m_options->popScreen)
    {
        m_options->popScreen();
    }
}

void Menu::addText(uint8_t id, const std::string &text)
{
    addTextCounter(id, text, 0);
}

void Menu::addTextCounter(uint8_t id, const std::string &text, const uint8_t value)
{
    auto callback = [this](const MenuItem &menuItem, const ButtonType button) -> void {
        onButtonPressed(menuItem, button);
    };
    if (value > 0)
    {
        m_items.emplace_back(id, MenuItemTypes::TEXT_COUNTER, text, std::to_string(value), callback);
    }
    else
    {
        m_items.emplace_back(id, MenuItemTypes::TEXT, text, callback);
    }
}

void Menu::addSelection(uint8_t id, const std::string &text, const std::vector<std::string> &values, const int index)
{
    auto callback = [this](const MenuItem &menuItem, const ButtonType button) -> void {
        onButtonPressed(menuItem, button);
    };
    m_items.emplace_back(id, MenuItemTypes::SELECTION, text, values, index, callback);
}

void Menu::addToggle(uint8_t id, const std::string &text, const bool selected)
{
    auto callback = [this](const MenuItem &menuItem, const ButtonType button) -> void {
        onButtonPressed(menuItem, button);
    };
    m_items.emplace_back(id, MenuItemTypes::TOGGLE, text, std::to_string(selected), callback);
}

void Menu::drawScrollBar() const
{
    // Create scrollbar instance
    ScrollBar scrollBar(m_options, u8g2->width - UIConstants::SCROLLBAR_WIDTH, 3, 1, u8g2->height - 6);
    scrollBar.refresh(m_selected_item, m_items.size());
    scrollBar.render();
}

void Menu::drawSelectionBox() const
{
    // Calculate dimensions for the selection box
    const auto displayHeight = u8g2->height;
    const auto displayWidth = u8g2->width;
    const auto boxHeight = displayHeight / 3;
    const auto y = boxHeight * 2 - 2;
    const auto x = displayWidth - UIConstants::RIGHT_PADDING;

    // Draw the rounded frame for the selection box
    u8g2_DrawRFrame(u8g2, 2, boxHeight, displayWidth - UIConstants::RIGHT_PADDING, boxHeight,
                    UIConstants::CORNER_RADIUS);

    // Draw horizontal line separator
    u8g2_DrawLine(u8g2, 4, y, displayWidth - UIConstants::RIGHT_PADDING, y);

    // Draw vertical line on the right side
    u8g2_DrawLine(u8g2, x, y - boxHeight + 3, x, y - 1);
}