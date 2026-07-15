/**
 * @file    ResourceCache.cpp
 * @brief   ResourceCache implementation.
 */

#include <Resources/ResourceCache.hpp>

namespace DragonOS::Resources {

void ResourceCache::Add(Resource::ID id, std::shared_ptr<Resource> resource) noexcept
{
    m_cache[id] = std::move(resource);
}

void ResourceCache::Remove(Resource::ID id) noexcept
{
    m_cache.erase(id);
}

void ResourceCache::Clear() noexcept
{
    m_cache.clear();
}

bool ResourceCache::Exists(Resource::ID id) const noexcept
{
    return m_cache.find(id) != m_cache.end();
}

std::shared_ptr<Resource> ResourceCache::Find(Resource::ID id) const noexcept
{
    const auto it = m_cache.find(id);
    return it != m_cache.end() ? it->second : nullptr;
}

} // namespace DragonOS::Resources
