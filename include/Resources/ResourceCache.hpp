/**
 * @file    ResourceCache.hpp
 * @brief   Internal cache for loaded Resource objects.
 *
 * ResourceCache manages an unordered_map of shared_ptr<Resource>
 * keyed by Resource::ID.  It provides basic CRUD operations and
 * is owned by the ResourceManager.
 *
 * Full implementation (eviction policies, LRU, etc.) belongs to a
 * later phase.
 */

#pragma once

#include <Resources/Resource.hpp>

#include <cstddef>
#include <memory>
#include <unordered_map>

namespace DragonOS::Resources {

/**
 * @brief  O(1)-average storage for loaded resources.
 *
 * Maintains a mapping  Resource::ID -> shared_ptr<Resource>  so the
 * ResourceManager can quickly look up, iterate, and evict resources.
 *
 * @note  This class provides the interface only.  Advanced caching
 *        strategies (LRU, TTL, budget enforcement) are deferred.
 */
class ResourceCache final {
public:
    ResourceCache() noexcept = default;
    ~ResourceCache() noexcept = default;

    ResourceCache(const ResourceCache&)            = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&)                 = delete;
    ResourceCache& operator=(ResourceCache&&)      = delete;

    // ── Mutators ─────────────────────────────────────────────────────────

    /**
     * @brief  Insert or update a resource in the cache.
     *
     * @param id        Resource identifier (must match resource->GetId()).
     * @param resource  Shared pointer to the resource.
     */
    void Add(Resource::ID id, std::shared_ptr<Resource> resource) noexcept;

    /**
     * @brief  Remove a resource from the cache by ID.
     *
     * Does nothing if the ID does not exist.
     *
     * @param id  The resource to remove.
     */
    void Remove(Resource::ID id) noexcept;

    /// @brief  Remove every entry from the cache.
    void Clear() noexcept;

    // ── Queries ──────────────────────────────────────────────────────────

    /**
     * @brief  Check whether an ID is present in the cache.
     *
     * @param id  Resource identifier.
     *
     * @return true if the cache contains this ID.
     */
    [[nodiscard]] bool Exists(Resource::ID id) const noexcept;

    /**
     * @brief  Look up a resource by ID.
     *
     * @param id  Resource identifier.
     *
     * @return Shared pointer to the resource, or nullptr.
     */
    [[nodiscard]] std::shared_ptr<Resource> Find(Resource::ID id) const noexcept;

    /// @brief  Number of entries currently in the cache.
    [[nodiscard]] std::size_t Size() const noexcept { return m_cache.size(); }

private:
    std::unordered_map<Resource::ID, std::shared_ptr<Resource>> m_cache;
};

} // namespace DragonOS::Resources
