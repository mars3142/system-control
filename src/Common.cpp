#include "Common.h"

#include <SDL3/SDL.h>

auto createWindow(const char *title, const int width, const int height) -> Window * {

    constexpr uint32_t window_flag = SDL_WINDOW_HIDDEN;
    const auto window = SDL_CreateWindow(title, width, height, window_flag);
    if (window == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window", SDL_GetError(), nullptr);
        return nullptr;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
    SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER, SDL_COLORSPACE_SRGB_LINEAR);
    if (const auto renderer = SDL_CreateRendererWithProperties(props); renderer == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create renderer", SDL_GetError(), nullptr);
        return nullptr;
    }

    return new Window(window);
}
