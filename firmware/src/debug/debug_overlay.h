#pragma once

#include <SDL3/SDL.h>

#include "model/AppContext.h"

namespace DebugOverlay
{
inline bool show_debug_window = false;
inline bool show_unhandled_events = false;
inline bool show_led_matrix = false;

constexpr auto width = 8;
constexpr auto height = 8;

void Init(const AppContext *context);

void Update(AppContext *context, const SDL_Event *event);

void Render(const AppContext *context);

void Cleanup();
} // namespace DebugOverlay