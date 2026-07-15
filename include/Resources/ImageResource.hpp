/**
 * @file    ImageResource.hpp
 * @brief   Represents a raster image resource (loaded via WIC in a later phase).
 *
 * Currently a skeleton -- the Load / Unload implementations are TODO
 * stubs and will be filled when the WIC loading subsystem is built.
 */

#pragma once

#include <Resources/Resource.hpp>

namespace DragonOS::Resources {

/**
 * @brief  A raster image resource (PNG, JPEG, BMP, ...).
 *
 * ImageResource owns a WIC bitmap and a D2D bitmap once loaded.
 * Actual loading logic belongs to a later phase.
 */
class ImageResource final : public Resource {
public:
    /**
     * @brief  Construct an image resource stub.
     *
     * @param path  Relative path to the image file.
     * @param id    Unique resource identifier.
     */
    explicit ImageResource(std::string path, ID id) noexcept
        : Resource(ResourceType::Image, std::move(path), id)
    {
    }

    /// @brief  TODO:  Load the image via WIC and create a D2D bitmap.
    [[nodiscard]] bool Load() override;

    /// @brief  TODO:  Release the WIC bitmap and D2D bitmap.
    void Unload() noexcept override;
};

} // namespace DragonOS::Resources
