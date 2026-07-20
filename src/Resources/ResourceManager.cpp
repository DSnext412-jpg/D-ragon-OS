/**
 * @file    ResourceManager.cpp
 * @brief   ResourceManager stub implementation.
 *
 * Actual resource loading, unloading, and tracking is implemented
 * in later phases.  Current phase establishes the interface only.
 */

#include <Resources/ResourceManager.hpp>

namespace DragonOS::Resources {

bool ResourceManager::Initialize() noexcept
{
    // TODO:  Register built-in loaders (ImageLoader, FontLoader, ...)
    m_initialized = true;
    return true;
}

void ResourceManager::Shutdown() noexcept
{
    Clear();
    m_initialized = false;
}

void ResourceManager::Update(float /*deltaTime*/) noexcept
{
    // TODO:  Process deferred unloads, LRU eviction, streaming I/O
}

ResourceHandle<Resource> ResourceManager::Load(
    ResourceType        /*type*/,
    std::string_view    /*path*/) noexcept
{
    // TODO:  Create the appropriate concrete Resource, call Load(),
    //        store in m_cache, and return a handle.
    return {};
}

void ResourceManager::Unload(Resource::ID id) noexcept
{
    const auto resource = m_cache.Find(id);
    if (resource)
    {
        resource->Unload();
        m_cache.Remove(id);
    }
}

bool ResourceManager::Reload(Resource::ID id) noexcept
{
    const auto resource = m_cache.Find(id);
    if (resource)
    {
        return resource->Reload();
    }
    return false;
}

ResourceHandle<Resource> ResourceManager::Get(Resource::ID id) const noexcept
{
    const auto resource = m_cache.Find(id);
    if (resource)
    {
        return ResourceHandle<Resource>{ resource };
    }
    return {};
}

void ResourceManager::Remove(Resource::ID id) noexcept
{
    m_cache.Remove(id);
}

void ResourceManager::Clear() noexcept
{
    // TODO:  Iterate and call Unload() on each entry before clearing.
    m_cache.Clear();
}

} // namespace DragonOS::Resources
