#pragma once
#include <unordered_map>
#include <string>
#include <mutex>

#include <SDL3/SDL.h>

class ResourceManager {
public:
    static ResourceManager& getInstance();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    ~ResourceManager();

    SDL_Texture* get_texture(SDL_Renderer* renderer, const std::string& path);

private:
    ResourceManager();

    std::unordered_map<std::string, SDL_Texture*> m_textures;

    mutable std::mutex m_mutex;
};
