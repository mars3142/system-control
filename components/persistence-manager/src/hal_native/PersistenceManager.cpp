#include "hal_native/PersistenceManager.h"
#include <SDL3/SDL.h>

#include <utility>

PersistenceManager::PersistenceManager(std::string filename) : m_filename(std::move(filename))
{
    if (!SDL_WasInit(SDL_INIT_EVENTS))
    {
        SDL_Init(SDL_INIT_EVENTS);
    }
}

PersistenceManager::~PersistenceManager()
{
    Save();
}

bool PersistenceManager::HasKey(const std::string &key) const
{
    return m_data.contains(key);
}

void PersistenceManager::RemoveKey(const std::string &key)
{
    m_data.erase(key);
}

void PersistenceManager::Clear()
{
    m_data.clear();
}

bool PersistenceManager::Save()
{
    return SaveToFile(m_filename);
}

bool PersistenceManager::Load()
{
    return LoadFromFile(m_filename);
}

bool PersistenceManager::SaveToFile(const std::string &filename)
{
    SDL_IOStream *stream = SDL_IOFromFile(filename.c_str(), "wb");
    if (!stream)
    {
        SDL_Log("Error opening file for writing: %s", SDL_GetError());
        return false;
    }

    const size_t count = m_data.size();
    if (SDL_WriteIO(stream, &count, sizeof(count)) != sizeof(count))
    {
        SDL_Log("Error writing count: %s", SDL_GetError());
        SDL_CloseIO(stream);
        return false;
    }

    for (const auto &[key, value] : m_data)
    {
        size_t keyLength = key.length();
        if (SDL_WriteIO(stream, &keyLength, sizeof(keyLength)) != sizeof(keyLength))
        {
            SDL_Log("Error writing key length: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        if (SDL_WriteIO(stream, key.c_str(), keyLength) != keyLength)
        {
            SDL_Log("Error writing key: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        if (!WriteValueToStream(stream, value))
        {
            SDL_CloseIO(stream);
            return false;
        }
    }

    SDL_CloseIO(stream);
    return true;
}

bool PersistenceManager::LoadFromFile(const std::string &filename)
{
    SDL_IOStream *stream = SDL_IOFromFile(filename.c_str(), "rb");
    if (!stream)
    {
        SDL_Log("File not found or error opening: %s", SDL_GetError());
        return false;
    }

    m_data.clear();

    size_t count;
    if (SDL_ReadIO(stream, &count, sizeof(count)) != sizeof(count))
    {
        SDL_Log("Error reading count: %s", SDL_GetError());
        SDL_CloseIO(stream);
        return false;
    }

    for (size_t i = 0; i < count; ++i)
    {
        size_t keyLength;
        if (SDL_ReadIO(stream, &keyLength, sizeof(keyLength)) != sizeof(keyLength))
        {
            SDL_Log("Error reading key length: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        std::string key(keyLength, '\0');
        if (SDL_ReadIO(stream, key.data(), keyLength) != keyLength)
        {
            SDL_Log("Error reading key: %s", SDL_GetError());
            SDL_CloseIO(stream);
            return false;
        }

        ValueType value;
        if (!ReadValueFromStream(stream, value))
        {
            SDL_CloseIO(stream);
            return false;
        }

        m_data[key] = value;
    }

    SDL_CloseIO(stream);
    return true;
}

void PersistenceManager::SetValueImpl(const std::string &key, bool value)
{
    m_data[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string &key, int value)
{
    m_data[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string &key, float value)
{
    m_data[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string &key, double value)
{
    m_data[key] = value;
}

void PersistenceManager::SetValueImpl(const std::string &key, const std::string &value)
{
    m_data[key] = value;
}

bool PersistenceManager::GetValueImpl(const std::string &key, bool defaultValue) const
{
    if (const auto it = m_data.find(key); it != m_data.end() && std::holds_alternative<bool>(it->second))
    {
        return std::get<bool>(it->second);
    }
    return defaultValue;
}

int PersistenceManager::GetValueImpl(const std::string &key, int defaultValue) const
{
    if (const auto it = m_data.find(key); it != m_data.end() && std::holds_alternative<int>(it->second))
    {
        return std::get<int>(it->second);
    }
    return defaultValue;
}

float PersistenceManager::GetValueImpl(const std::string &key, float defaultValue) const
{
    if (const auto it = m_data.find(key); it != m_data.end() && std::holds_alternative<float>(it->second))
    {
        return std::get<float>(it->second);
    }
    return defaultValue;
}

double PersistenceManager::GetValueImpl(const std::string &key, double defaultValue) const
{
    if (const auto it = m_data.find(key); it != m_data.end() && std::holds_alternative<double>(it->second))
    {
        return std::get<double>(it->second);
    }
    return defaultValue;
}

std::string PersistenceManager::GetValueImpl(const std::string &key, const std::string &defaultValue) const
{
    if (const auto it = m_data.find(key); it != m_data.end() && std::holds_alternative<std::string>(it->second))
    {
        return std::get<std::string>(it->second);
    }
    return defaultValue;
}

bool PersistenceManager::WriteValueToStream(SDL_IOStream *stream, const ValueType &value)
{
    const TypeId typeId = GetTypeId(value);

    if (SDL_WriteIO(stream, &typeId, sizeof(typeId)) != sizeof(typeId))
    {
        SDL_Log("Error writing type ID: %s", SDL_GetError());
        return false;
    }

    switch (typeId)
    {
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
        const auto &str = std::get<std::string>(value);
        const size_t length = str.length();

        if (SDL_WriteIO(stream, &length, sizeof(length)) != sizeof(length))
        {
            return false;
        }

        return SDL_WriteIO(stream, str.c_str(), length) == length;
    }
    }
    return false;
}

bool PersistenceManager::ReadValueFromStream(SDL_IOStream *stream, ValueType &value)
{
    TypeId typeId;

    if (SDL_ReadIO(stream, &typeId, sizeof(typeId)) != sizeof(typeId))
    {
        SDL_Log("Error reading type ID: %s", SDL_GetError());
        return false;
    }

    switch (typeId)
    {
    case TypeId::BOOL: {
        bool val;
        if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val))
        {
            value = val;
            return true;
        }
        break;
    }
    case TypeId::INT: {
        int val;
        if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val))
        {
            value = val;
            return true;
        }
        break;
    }
    case TypeId::FLOAT: {
        float val;
        if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val))
        {
            value = val;
            return true;
        }
        break;
    }
    case TypeId::DOUBLE: {
        double val;
        if (SDL_ReadIO(stream, &val, sizeof(val)) == sizeof(val))
        {
            value = val;
            return true;
        }
        break;
    }
    case TypeId::STRING: {
        size_t length;
        if (SDL_ReadIO(stream, &length, sizeof(length)) != sizeof(length))
        {
            return false;
        }

        std::string str(length, '\0');
        if (SDL_ReadIO(stream, str.data(), length) == length)
        {
            value = str;
            return true;
        }
        break;
    }
    }

    SDL_Log("Error reading value: %s", SDL_GetError());
    return false;
}

PersistenceManager::TypeId PersistenceManager::GetTypeId(const ValueType &value)
{
    if (std::holds_alternative<bool>(value))
        return TypeId::BOOL;
    if (std::holds_alternative<int>(value))
        return TypeId::INT;
    if (std::holds_alternative<float>(value))
        return TypeId::FLOAT;
    if (std::holds_alternative<double>(value))
        return TypeId::DOUBLE;
    if (std::holds_alternative<std::string>(value))
        return TypeId::STRING;

    return TypeId::BOOL;
}