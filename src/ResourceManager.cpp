#include "ResourceManager.h"

#include <memory>
#include <iostream>
#include <SDL3_image/SDL_image.h>

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::ResourceManager() {
    SDL_Log("ResourceManager instance created.");
}

ResourceManager::~ResourceManager() {
    for(const auto& pair : m_textures) {
        if(pair.second != nullptr) {
            SDL_DestroyTexture(pair.second);
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
        std::cerr << "ResourceManager Fehler: Konnte Textur nicht laden '" << path << std::endl;
        return nullptr;
    }

    m_textures[path] = texture;
    return texture;
}

