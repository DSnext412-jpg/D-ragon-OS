/**
 * @file    ResourceManager.hpp
 * @brief   Central coordinator for all resource lifecycle operations.
 *
 * The ResourceManager provides a unified interface for loading,
 * unloading, reloading, and looking up resources.  It owns a
 * ResourceCache for storage and delegates to concrete resource
 * loaders (added in later phases).
 *
 * Integration with Engine::System happens in Phase 8C.
 */

#pragma once

#include <Resources/ResourceCache.hpp>
#include <Resources/ResourceHandle.hpp>
#include <Resources/ResourceType.hpp>

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace DragonOS::Resources {

/**
 * @brief  Manages the full lifecycle of all resources in DragonOS.
 *
 * ## Responsibilities (current phase -- interface only)
 *   - Own the ResourceCache.
 *   - Provide Initialize / Shutdown / Update lifecycle hooks.
 *   - Declare Load / Unload / Reload / Get / Remove / Clear.
 *
 * ## Responsibilities (future phases)
 *   - Register concrete loaders (ImageLoader, FontLoader, ...).
 *   - Dispatch Load() to the correct loader based on ResourceType.
 *   - Track load dependencies and reference counts.
 */
class ResourceManager final {
public:
    ResourceManager() noexcept = default;
    ~ResourceManager() noexcept { Shutdown(); }

    ResourceManager(const ResourceManager&)            = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&)                 = delete;
    ResourceManager& operator=(ResourceManager&&)      = delete;

    // ── Lifecycle ────────────────────────────────────────────────────────

    /**
     * @brief  Prepare the resource manager for operation.
     *
     * TODO:  Register built-in loaders (Phase 8B).
     *
     * @return true on success.
     */
    [[nodiscard]] bool Initialize() noexcept;

    /// @brief  Unload all resources and release internal state.
    void Shutdown() noexcept;

    /**
     * @brief  Per-frame housekeeping.
     *
     * TODO:  Process deferred unloads, LRU eviction, streaming I/O.
     *
     * @param deltaTime  Seconds elapsed since the previous frame.
     */
    void Update(float deltaTime) noexcept;

    // ── Resource lifecycle ───────────────────────────────────────────────

    /**
     * @brief  Load a resource by type and path.
     *
     * TODO:  Implement actual loading (Phase 8B+).
     *
     * @param type  The kind of resource to load.
     * @param path  Relative file path.
     *
     * @return A handle to the loaded resource, or an invalid handle on failure.
     */
    [[nodiscard]] ResourceHandle<Resource> Load(
        ResourceType        type,
        std::string_view    path) noexcept;

    /**
     * @brief  Unload a previously loaded resource.
     *
     * @param id  Identifier of the resource to unload.
     */
    void Unload(Resource::ID id) noexcept;

    /**
     * @brief  Reload a resource from disk.
     *
     * @param id  Identifier of the resource to reload.
     *
     * @return true if the reload succeeded.
     */
    [[nodiscard]] bool Reload(Resource::ID id) noexcept;

    // ── Lookup ───────────────────────────────────────────────────────────

    /**
     * @brief  Retrieve a handle to a loaded resource by ID.
     *
     * @param id  Resource identifier.
     *
     * @return A handle that may be valid or invalid.
     */
    [[nodiscard]] ResourceHandle<Resource> Get(Resource::ID id) const noexcept;

    /**
     * @brief  Remove a resource from management without unloading.
     *
     * @param id  Identifier of the resource to remove.
     */
    void Remove(Resource::ID id) noexcept;

    /// @brief  Unload and remove every managed resource.
    void Clear() noexcept;

private:
    ResourceCache m_cache;
    bool m_initialized{ false };
};

} // namespace DragonOS::Resources
