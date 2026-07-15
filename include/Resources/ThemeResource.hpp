/**
 * @file    ThemeResource.hpp
 * @brief   Represents a visual theme resource.
 *
 * Skeleton -- theme parsing will be implemented in a later phase.
 */

#pragma once

#include <Resources/Resource.hpp>

namespace DragonOS::Resources {

/**
 * @brief  A visual theme defining colours, metrics, and styles.
 *
 * ThemeResource holds parsed colour palettes, size constants, and
 * style definitions.  Actual parsing (JSON, XML, or custom format)
 * belongs to a later phase.
 */
class ThemeResource final : public Resource {
public:
    /**
     * @brief  Construct a theme resource stub.
     *
     * @param path  Relative path to the theme definition file.
     * @param id    Unique resource identifier.
     */
    explicit ThemeResource(std::string path, ID id) noexcept
        : Resource(ResourceType::Theme, std::move(path), id)
    {
    }

    /// @brief  TODO:  Parse the theme file and populate colour/style tables.
    [[nodiscard]] bool Load() override;

    /// @brief  TODO:  Clear parsed theme data.
    void Unload() noexcept override;
};

} // namespace DragonOS::Resources
