/**
 * @file    IconResource.hpp
 * @brief   Represents a window or shell icon resource.
 *
 * Skeleton -- icon loading will be implemented in a later phase.
 */

#pragma once

#include <Resources/Resource.hpp>

namespace DragonOS::Resources {

/**
 * @brief  A draggable or display icon (loaded from .ico, .png, or
 *         embedded resource).
 *
 * IconResource wraps a HICON handle once loaded.
 */
class IconResource final : public Resource {
public:
    /**
     * @brief  Construct an icon resource stub.
     *
     * @param path  Relative path to the icon file.
     * @param id    Unique resource identifier.
     */
    explicit IconResource(std::string path, ID id) noexcept
        : Resource(ResourceType::Icon, std::move(path), id)
    {
    }

    /// @brief  TODO:  Load the icon from disk and create a HICON.
    [[nodiscard]] bool Load() override;

    /// @brief  TODO:  Destroy the HICON handle.
    void Unload() noexcept override;
};

} // namespace DragonOS::Resources
