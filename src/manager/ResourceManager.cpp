#include "ResourceManager.h"

#include <SDL3_image/SDL_image.h>
#include <memory>
#include <ranges>

ResourceManager &ResourceManager::Instance()
{
    static ResourceManager instance;
    return instance;
}

ResourceManager::ResourceManager()
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ResourceManager instance created.");
}

/**
 * @brief Destructor for the ResourceManager
 * Cleans up all loaded textures and frees memory
 */
ResourceManager::~ResourceManager()
{
    for (const auto &texture : m_textures | std::views::values)
    {
        if (texture != nullptr)
        {
            SDL_DestroyTexture(texture);
        }
    }
    m_textures.clear();
}

std::string ResourceManager::GetResourcePath(const std::string &fileName)
{
    const auto basePath_c = SDL_GetBasePath();
    if (!basePath_c)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error retrieving base path: %s", SDL_GetError());
        // Fallback to relative path as a last resort
        // Warning: This only works reliably if the working directory is correct
        // (e.g., when debugging from IDE)
        return fileName; // Fallback (unsafe)
    }

    const std::string basePath(basePath_c);

    std::string fullPath = basePath;
#ifdef __APPLE__
    // Special handling for macOS bundle structure
    // Navigate from /Contents/MacOS/ to /Contents/Resources/
    size_t lastSlash = fullPath.find_last_of('/');
    if (lastSlash != std::string::npos)
    {
        fullPath = fullPath.substr(0, lastSlash); // Remove executable name
        lastSlash = fullPath.find_last_of('/');
        if (lastSlash != std::string::npos)
        {
            fullPath = fullPath.substr(0, lastSlash + 1); // Go to Contents folder
            fullPath += "Resources/";
        }
        else
        {
            // Fallback if structure is unexpected
            fullPath = basePath; // Use original base path
            fullPath += "Resources/"; // And try with this
        }
    }
    else
    {
        // Fallback for unexpected path structure
        fullPath = basePath;
        fullPath += "Resources/";
    }
#else
    // For other platforms (Windows, Linux, etc.)
    // Use standard assets folder
    fullPath += "assets/";
#endif

    return fullPath + fileName;
}


SDL_Texture *ResourceManager::GetTextureByName(SDL_Renderer *renderer, const std::string &path)
{
    std::lock_guard lock(m_mutex);

    // Check if texture is already in cache
    if (const auto search = m_textures.find(GetResourcePath(path)); search != m_textures.end())
    {
        return search->second;
    }

    // Load new texture
    SDL_Texture *texture = IMG_LoadTexture(renderer, GetResourcePath(path).c_str());

    if (!texture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load %s -> %s", GetResourcePath(path).c_str(),
                     SDL_GetError());
        return nullptr;
    }

    // Store texture in cache
    m_textures[path] = texture;
    return texture;
}
