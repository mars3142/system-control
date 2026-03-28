#include "hermes/hermes.h"
#include "hermes/screensaver/clock_screensaver.h"
#include "hermes/screensaver/screensaver.h"
#include "mercedes/mercedes.h"

#include <esp_log.h>

static const char *TAG = "Hermes";

// UI layout constants
namespace UI
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
} // namespace UI

// Renderer state
enum class RenderState
{
    SPLASH,
    MENU,
    SCREENSAVER
};

static u8g2_t *s_u8g2 = nullptr;
static RenderState s_state = RenderState::SPLASH;

// Inactivity tracking
static uint32_t s_inactivity_timeout_ms = 0;
static uint64_t s_inactivity_elapsed = 0;

static Screensaver *s_screensaver = nullptr;

// ============================================================================
// Scrollbar
// ============================================================================

static void draw_scrollbar(size_t current, size_t total)
{
    if (total <= 1)
        return;

    int x = s_u8g2->width - UI::SCROLLBAR_WIDTH;
    int trackHeight = s_u8g2->height - 6;
    int trackY = 3;

    for (int y = trackY; y < trackY + trackHeight; y += 2)
    {
        u8g2_DrawPixel(s_u8g2, x + 1, y);
    }

    int thumbHeight = trackHeight / static_cast<int>(total);
    if (thumbHeight < 3)
        thumbHeight = 3;
    int thumbY = trackY + static_cast<int>(current) * (trackHeight - thumbHeight) / static_cast<int>(total - 1);
    u8g2_DrawBox(s_u8g2, x, thumbY, UI::SCROLLBAR_WIDTH, thumbHeight);
}

// ============================================================================
// Selection box
// ============================================================================

static void draw_selection_box()
{
    int h = s_u8g2->height;
    int w = s_u8g2->width;
    int boxHeight = h / 3;
    int y = boxHeight * 2 - 2;
    int x = w - UI::RIGHT_PADDING;

    u8g2_DrawRFrame(s_u8g2, 2, boxHeight, w - UI::RIGHT_PADDING, boxHeight, UI::CORNER_RADIUS);
    u8g2_DrawLine(s_u8g2, 4, y, w - UI::RIGHT_PADDING, y);
    u8g2_DrawLine(s_u8g2, x, y - boxHeight + 3, x, y - 1);
}

// ============================================================================
// Item rendering
// ============================================================================

static void draw_item(const MenuItemDef &item, const uint8_t *font, int x, int y)
{
    u8g2_SetFont(s_u8g2, font);
    u8g2_DrawStr(s_u8g2, x, y, item.label.c_str());

    if (item.type == "label")
    {
        auto &menu = Mercedes::getInstance();
        const auto *provider = menu.getItemValueProvider();
        if (provider)
        {
            static char val[32];
            val[0] = '\0';
            (*provider)(item.id, val, sizeof(val));
            if (val[0] != '\0')
            {
                u8g2_uint_t tw = u8g2_GetStrWidth(s_u8g2, val);
                u8g2_DrawStr(s_u8g2, s_u8g2->width - tw - UI::SELECTION_MARGIN, y, val);
            }
        }
    }
    else if (item.type == "submenu" || item.type == "action")
    {
        const char *arrow = ">";
        u8g2_uint_t tw = u8g2_GetStrWidth(s_u8g2, arrow);
        u8g2_DrawStr(s_u8g2, s_u8g2->width - tw - UI::SELECTION_MARGIN, y, arrow);
    }
    else if (item.type == "selection" && !item.selectionItems.empty())
    {
        int idx = item.selectionIndex;
        if (idx >= 0 && idx < static_cast<int>(item.selectionItems.size()))
        {
            static char formatted[48];
            snprintf(formatted, sizeof(formatted), "< %s >", item.selectionItems[idx].label.c_str());
            u8g2_uint_t tw = u8g2_GetStrWidth(s_u8g2, formatted);
            u8g2_DrawStr(s_u8g2, s_u8g2->width - tw - UI::SELECTION_MARGIN, y, formatted);
        }
    }
    else if (item.type == "toggle")
    {
        int frameX = s_u8g2->width - UI::FRAME_BOX_SIZE - UI::SELECTION_MARGIN;
        int frameY = y - UI::FRAME_OFFSET;
        u8g2_DrawFrame(s_u8g2, frameX, frameY, UI::FRAME_BOX_SIZE, UI::FRAME_BOX_SIZE);

        if (item.toggleValue)
        {
            int x1 = frameX + 2;
            int y1 = frameY + 2;
            int x2 = frameX + UI::FRAME_BOX_SIZE - 3;
            int y2 = frameY + UI::FRAME_BOX_SIZE - 3;
            u8g2_DrawLine(s_u8g2, x1, y1, x2, y2);
            u8g2_DrawLine(s_u8g2, x1, y2, x2, y1);
        }
    }
}

// ============================================================================
// Splash screen
// ============================================================================

static void draw_splash()
{
    u8g2_SetFont(s_u8g2, u8g2_font_DigitalDisco_tr);
    u8g2_DrawStr(s_u8g2, 28, s_u8g2->height / 2 - 10, "HO Anlage");
    u8g2_DrawStr(s_u8g2, 30, s_u8g2->height / 2 + 5, "Axel Janz");
    u8g2_SetFont(s_u8g2, u8g2_font_haxrcorp4089_tr);
    u8g2_DrawStr(s_u8g2, 35, 50, "Initialisierung...");
}

// ============================================================================
// Menu screen
// ============================================================================

static void draw_menu()
{
    auto &menu = Mercedes::getInstance();
    const MenuScreenDef *screen = menu.getCurrentScreen();
    if (!screen || screen->items.empty())
        return;

    size_t selected = menu.getSelectedIndex();

    static size_t visibleIdx[32];
    size_t visibleCount = 0;
    size_t visibleSelected = 0;

    for (size_t i = 0; i < screen->items.size(); i++)
    {
        if (menu.isItemVisible(screen->items[i]))
        {
            if (i == selected)
                visibleSelected = visibleCount;
            visibleIdx[visibleCount++] = i;
        }
    }

    if (visibleCount == 0)
        return;

    draw_scrollbar(visibleSelected, visibleCount);
    draw_selection_box();

    int centerY = s_u8g2->height / 2 + 3;
    draw_item(screen->items[visibleIdx[visibleSelected]], u8g2_font_helvB08_tr, UI::LEFT_MARGIN, centerY);

    if (visibleSelected > 0)
        draw_item(screen->items[visibleIdx[visibleSelected - 1]], u8g2_font_haxrcorp4089_tr, UI::LEFT_MARGIN, UI::LINE_SPACING);

    if (visibleSelected < visibleCount - 1)
        draw_item(screen->items[visibleIdx[visibleSelected + 1]], u8g2_font_haxrcorp4089_tr, UI::LEFT_MARGIN,
                  s_u8g2->height - UI::BOTTOM_OFFSET);
}

// ============================================================================
// Public API
// ============================================================================

extern "C" void hermes_init(u8g2_t *u8g2, uint32_t inactivity_timeout_ms)
{
    s_u8g2 = u8g2;
    s_inactivity_timeout_ms = inactivity_timeout_ms;
    s_inactivity_elapsed = 0;
    s_state = RenderState::SPLASH;
    s_screensaver = new ClockScreensaver(u8g2);
    ESP_LOGI(TAG, "Hermes initialized (screensaver timeout: %lu ms)", (unsigned long)inactivity_timeout_ms);
}

extern "C" void hermes_draw(uint64_t dt)
{
    if (!s_u8g2)
        return;

    // Clear
    u8g2_SetDrawColor(s_u8g2, 0);
    u8g2_DrawBox(s_u8g2, 0, 0, s_u8g2->width, s_u8g2->height);
    u8g2_SetDrawColor(s_u8g2, 1);

    // Determine state transitions
    auto &menu = Mercedes::getInstance();
    const MenuScreenDef *screen = menu.getCurrentScreen();
    bool hasItems = (screen && !screen->items.empty());

    if (s_state == RenderState::SPLASH && hasItems)
    {
        s_state = RenderState::MENU;
        s_inactivity_elapsed = 0;
    }

    // Inactivity tracking (only in MENU state)
    if (s_state == RenderState::MENU && s_inactivity_timeout_ms > 0)
    {
        s_inactivity_elapsed += dt;
        if (s_inactivity_elapsed >= s_inactivity_timeout_ms)
        {
            s_state = RenderState::SCREENSAVER;
            s_screensaver->init();
        }
    }

    // Draw current state
    switch (s_state)
    {
    case RenderState::SPLASH:
        draw_splash();
        break;
    case RenderState::MENU:
        draw_menu();
        break;
    case RenderState::SCREENSAVER:
        s_screensaver->draw(dt);
        break;
    }
}

extern "C" void hermes_reset_inactivity(void)
{
    s_inactivity_elapsed = 0;
    if (s_state == RenderState::SCREENSAVER)
    {
        s_state = RenderState::MENU;
    }
}
