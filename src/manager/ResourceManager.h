#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include <SDL3/SDL.h>

class ResourceManager
{
public:
    /**
     * @brief Returns the singleton instance of the ResourceManager
     * @return Reference to the ResourceManager instance
     */
    static ResourceManager &Instance();

    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
    ResourceManager(ResourceManager &&) = delete;
    ResourceManager &operator=(ResourceManager &&) = delete;

    ~ResourceManager();

    /**
     * @brief Loads a texture or returns an already loaded texture
     * @param renderer The SDL_Renderer used to load the texture
     * @param path Path to the texture file (relative to resource directory)
     * @return Pointer to the loaded SDL_Texture or nullptr on error
     *
     * This function implements a caching system for textures. If a texture
     * has already been loaded, it returns it from the cache. Otherwise,
     * it loads the texture and stores it in the cache. Access is thread-safe
     * through mutex protection.
     */
    SDL_Texture *GetTextureByName(SDL_Renderer *renderer, const std::string &path);

private:
    /**
     * @brief Constructor for the ResourceManager
     * Initializes a new instance and logs its creation
     */
    ResourceManager();

    /**
     * @brief Determines the full resource path for a file
     * @param fileName Name of the resource file
     * @return Complete path to the resource
     *
     * This function handles platform-specific paths (especially macOS) and
     * constructs the correct path to resources. For macOS, it considers the
     * bundle structure (.app/Contents/Resources/).
     */
    static std::string GetResourcePath(const std::string &fileName);

    std::unordered_map<std::string, SDL_Texture *> m_textures;

    mutable std::mutex m_mutex;
};