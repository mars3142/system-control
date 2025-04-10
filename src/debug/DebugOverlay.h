#pragma once

#include <SDL3/SDL.h>

#include "model/AppContext.h"

namespace DebugOverlay {
inline bool show_debug_window = false;
inline bool show_unhandled_events = false;
inline bool show_led_matrix = true;

void init(const AppContext* context);

void update(AppContext* context, const SDL_Event* event);

void render(const AppContext* context);

void cleanup();
}
