#include "PersistenceManager.h"
#include <SDL3/SDL.h>

#include <utility>

#ifndef ESP_PLATFORM
PersistenceManager::PersistenceManager(std::string filename)
    : filename_(std::move(filename)) {
    // Initialize SDL3 if not already done
    if (!SDL_WasInit(SDL_INIT_EVENTS)) {
        SDL_Init(SDL_INIT_EVENTS);
    }
}

PersistenceManager::~PersistenceManager() {
    Save(); // Automatically save on destruction
}

bool PersistenceManager::HasKey(const std::string& key) const {
    return data_.contains(key);
}

void PersistenceManager::RemoveKey(const std::string& key) {
    data_.erase(key);
}

void PersistenceManager::Clear() {
    data_.clear();
}

bool PersistenceManager::Save() {
    return SaveToFile(filename_);
}

bool PersistenceManager::Load() {
    return LoadFromFile(filename_);
}

bool PersistenceManager::SaveToFile(const std::string& filename) {
    SDL_IOStream* stream = SDL_IOFromFile(filename.c_str(), "wb");
    if (!stream) {
        SDL_Log("Error opening file for writing: %s", SDL_GetError());
        return false;
    }

    // Write number of entries
    const size_t count = data_.size();
    if (SDL_WriteIO(stream, &count, sizeof(count)) != sizeof(count)) {
        SDL_Log("Error writing count: %s", SDL_GetError());
        SDL_CloseIO(stream);
        return false;
    }

    // Write each entry
    for (const auto& [key, value] : data_) {
        // Write key length
        size_t keyLength = key.length();
        if (SDL_WriteIO(stream, &keyLength, sizeof(keyLength)) != sizeof(keyLength)) {
            SDL_Log("Error writing key length: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        // Write key
        if (SDL_WriteIO(stream, key.c_str(), keyLength) != keyLength) {
            SDL_Log("Error writing key: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        // Write value
        if (!WriteValueToStream(stream, value)) {
            SDL_CloseIO(stream);
            return false;
        }
    }

    SDL_CloseIO(stream);
    return true;
}

bool PersistenceManager::LoadFromFile(const std::string& filename) {
    SDL_IOStream* stream = SDL_IOFromFile(filename.c_str(), "rb");
    if (!stream) {
        SDL_Log("File not found or error opening: %s", SDL_GetError());
        return false;
    }

    data_.clear();

    // Read number of entries
    size_t count;
    if (SDL_ReadIO(stream, &count, sizeof(count)) != sizeof(count)) {
        SDL_Log("Error reading count: %s", SDL_GetError());
        SDL_CloseIO(stream);
        return false;
    }

    // Read each entry
    for (size_t i = 0; i < count; ++i) {
        // Read key length
        size_t keyLength;
        if (SDL_ReadIO(stream, &keyLength, sizeof(keyLength)) != sizeof(keyLength)) {
            SDL_Log("Error reading key length: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        // Read key
        std::string key(keyLength, '\0');
        if (SDL_ReadIO(stream, key.data(), keyLength) != keyLength) {
            SDL_Log("Error reading key: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        // Read value
        ValueType value;
        if (!ReadValueFromStream(stream, value)) {
            SDL_CloseIO(stream);
            return false;
        }

        data_[key] = value;
    }

    SDL_CloseIO(stream);
    return true;
}

// Template-specific implementations
void PersistenceManager::SetValueImpl(const std::string& key, bool value) {
    data_[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string& key, int value) {
    data_[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string& key, float value) {
    data_[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string& key, double value) {
    data_[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string& key, const std::string& value) {
    data_[key] = value;
}

bool PersistenceManager::GetValueImpl(const std::string& key, bool defaultValue) const {
    if (const auto it = data_.find(key); it != data_.end() && std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second);
    }
    return defaultValue;
}

int PersistenceManager::GetValueImpl(const std::string& key, int defaultValue) const {
    if (const auto it = data_.find(key); it != data_.end() && std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second);
    }
    return defaultValue;
}

float PersistenceManager::GetValueImpl(const std::string& key, float defaultValue) const {
    if (const auto it = data_.find(key); it != data_.end() && std::holds_alternative<float>(it->second)) {
        return std::get<float>(it->second);
    }
    return defaultValue;
}

double PersistenceManager::GetValueImpl(const std::string& key, double defaultValue) const {
    if (const auto it = data_.find(key); it != data_.end() && std::holds_alternative<double>(it->second)) {
        return std::get<double>(it->second);
    }
    return defaultValue;
}

std::string PersistenceManager::GetValueImpl(const std::string& key, const std::string& defaultValue) const {
    if (const auto it = data_.find(key); it != data_.end() && std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
    }
    return defaultValue;
}

// Private helper methods
bool PersistenceManager::WriteValueToStream(SDL_IOStream* stream, const ValueType& value) {
    const TypeId typeId = GetTypeId(value);
    
    // Write type ID
    if (SDL_WriteIO(stream, &typeId, sizeof(typeId)) != sizeof(typeId)) {
        SDL_Log("Error writing type ID: %s", SDL_GetError());
        return false;
    }

    // Write value based on type
    switch (typeId) {
        case TypeId::BOOL: {
            const bool val = std::get<bool>(value);
            return SDL_WriteIO(stream, &val, sizeof(val)) == sizeof(val);
        }
        case TypeId::INT: {
            const int val = std::get<int>(value);
            return SDL_WriteIO(stream, &val, sizeof(val)) == sizeof(val);
        }
        case TypeId::FLOAT: {
            const float val = std::get<float>(value);
            return SDL_WriteIO(stream, &val, sizeof(val)) == sizeof(val);
        }
        case TypeId::DOUBLE: {
            const double val = std::get<double>(value);
            return SDL_WriteIO(stream, &val, sizeof(val)) == sizeof(val);
        }
        case TypeId::STRING: {
            const auto& str = std::get<std::string>(value);
            const size_t length = str.length();
            
            // Write string length
            if (SDL_WriteIO(stream, &length, sizeof(length)) != sizeof(length)) {
                return false;
            }
            
            // Write string data
            return SDL_WriteIO(stream, str.c_str(), length) == length;
        }
    }
    return false;
}

bool PersistenceManager::ReadValueFromStream(SDL_IOStream* stream, ValueType& value) {
    TypeId typeId;
    
    // Read type ID
    if (SDL_ReadIO(stream, &typeId, sizeof(typeId)) != sizeof(typeId)) {
        SDL_Log("Error reading type ID: %s", SDL_GetError());
        return false;
    }

    // Read value based on type
    switch (typeId) {
        case TypeId::BOOL: {
            bool val;
            if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val)) {
                value = val;
                return true;
            }
            break;
        }
        case TypeId::INT: {
            int val;
            if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val)) {
                value = val;
                return true;
            }
            break;
        }
        case TypeId::FLOAT: {
            float val;
            if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val)) {
                value = val;
                return true;
            }
            break;
        }
        case TypeId::DOUBLE: {
            double val;
            if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val)) {
                value = val;
                return true;
            }
            break;
        }
        case TypeId::STRING: {
            size_t length;
            if (SDL_ReadIO(stream, &length, sizeof(length)) != sizeof(length)) {
                return false;
            }
            
            std::string str(length, '\0');
            if (SDL_ReadIO(stream, str.data(), length) == length) {
                value = str;
                return true;
            }
            break;
        }
    }
    
    SDL_Log("Error reading value: %s", SDL_GetError());
    return false;
}

PersistenceManager::TypeId PersistenceManager::GetTypeId(const ValueType& value)
{
    if (std::holds_alternative<bool>(value)) return TypeId::BOOL;
    if (std::holds_alternative<int>(value)) return TypeId::INT;
    if (std::holds_alternative<float>(value)) return TypeId::FLOAT;
    if (std::holds_alternative<double>(value)) return TypeId::DOUBLE;
    if (std::holds_alternative<std::string>(value)) return TypeId::STRING;
    
    // Should never be reached
    return TypeId::BOOL;
}
#endif // !ESP_PLATFORM