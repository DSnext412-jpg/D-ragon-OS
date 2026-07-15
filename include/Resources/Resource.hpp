/**
 * @file    Resource.hpp
 * @brief   Abstract base class for all loadable resources in DragonOS.
 *
 * Every concrete resource (ImageResource, IconResource, FontResource,
 * ThemeResource, etc.) derives from Resource and implements the
 * virtual Load / Unload / Reload lifecycle.
 */

#pragma once

#include <Resources/ResourceType.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace DragonOS::Resources {

/**
 * @brief  Base class for every managed resource.
 *
 * A Resource represents a loadable asset identified by a unique ID and
 * a file-system path.  The ResourceManager tracks all live resources
 * through their base-class pointer.
 *
 * ## Lifecycle
 *   Created    -> constructor (unloaded)
 *   Load()     -> moves to loaded state
 *   Unload()   -> returns to unloaded state
 *   Reload()   -> Unload + Load (convenience)
 */
class Resource {
public:
    /// @brief  Unique numeric identifier for this resource.
    using ID = std::uint64_t;

    /// @brief  Clock type used for creation timestamps.
    using Clock = std::chrono::steady_clock;

    /**
     * @brief  Construct a resource with its type, path, and ID.
     *
     * @param type  The kind of resource (Image, Icon, Font, ...).
     * @param path  File-system path relative to the asset root.
     * @param id    Unique identifier (must be unique across all resources).
     */
    explicit Resource(
        ResourceType        type,
        std::string         path,
        ID                  id) noexcept
        : m_type(type)
        , m_path(std::move(path))
        , m_id(id)
        , m_creationTime(Clock::now())
    {
    }

    virtual ~Resource() noexcept = default;

    Resource(const Resource&)            = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&)                 = delete;
    Resource& operator=(Resource&&)      = delete;

    // ── Lifecycle ────────────────────────────────────────────────────────

    /**
     * @brief  Load the resource into memory.
     *
     * Pure virtual -- each derived type implements its own loading logic.
     *
     * @return true on success, false on failure.
     */
    [[nodiscard]] virtual bool Load() = 0;

    /**
     * @brief  Unload the resource and free associated memory.
     *
     * After calling Unload(), IsLoaded() returns false and the resource
     * may be Load()'d again.
     */
    virtual void Unload() noexcept = 0;

    /**
     * @brief  Convenience: Unload then Load.
     *
     * Default implementation calls Unload() then Load().  Derived types
     * may override for more efficient hot-reload paths.
     *
     * @return true on success.
     */
    [[nodiscard]] virtual bool Reload()
    {
        Unload();
        return Load();
    }

    // ── Accessors ────────────────────────────────────────────────────────

    /// @brief  Unique identifier for this resource.
    [[nodiscard]] ID GetId()                const noexcept { return m_id; }

    /// @brief  File-system path (relative to asset root).
    [[nodiscard]] const std::string& GetPath()  const noexcept { return m_path; }

    /// @brief  The type of resource this object represents.
    [[nodiscard]] ResourceType GetType()        const noexcept { return m_type; }

    /// @brief  Whether the resource is currently loaded in memory.
    [[nodiscard]] bool IsLoaded()               const noexcept { return m_loaded; }

    /// @brief  File size in bytes (0 if not loaded).
    [[nodiscard]] std::uint64_t GetFileSize()   const noexcept { return m_fileSize; }

    /// @brief  Timestamp when this resource object was created.
    [[nodiscard]] Clock::time_point GetCreationTime() const noexcept { return m_creationTime; }

protected:
    ResourceType        m_type;            ///< Kind of resource.
    std::string         m_path;            ///< Relative file path.
    ID                  m_id;              ///< Unique identifier.
    bool                m_loaded{ false }; ///< Load state.
    std::uint64_t       m_fileSize{ 0 };   ///< Size on disk (bytes).
    Clock::time_point   m_creationTime;    ///< Object creation timestamp.
};

} // namespace DragonOS::Resources
