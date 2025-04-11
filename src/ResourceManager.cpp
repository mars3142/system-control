#include "ResourceManager.h"

#include <memory>
#include <iostream>
#include <SDL3_image/SDL_image.h>
#include <ranges>

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::ResourceManager() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ResourceManager instance created.");
}

ResourceManager::~ResourceManager() {
    for(const auto& texture : m_textures | std::views::values) {
        if(texture != nullptr) {
            SDL_DestroyTexture(texture);
        }
    }
    m_textures.clear();
}

SDL_Texture* ResourceManager::get_texture(SDL_Renderer* renderer, const std::string& path) {
    std::lock_guard lock(m_mutex);

    if(const auto search = m_textures.find(path); search != m_textures.end()) {
        return search->second;
    }

    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());

    if(!texture) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not load %s -> %s",
            path.c_str(),
            SDL_GetError());
        return nullptr;
    }

    m_textures[path] = texture;
    return texture;
}
