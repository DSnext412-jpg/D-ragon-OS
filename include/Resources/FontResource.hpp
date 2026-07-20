/**
 * @file    FontResource.hpp
 * @brief   Represents a font resource (loaded via DirectWrite in a later phase).
 *
 * Skeleton -- font loading will be implemented in a later phase.
 */

#pragma once

#include <Resources/Resource.hpp>

namespace DragonOS::Resources {

/**
 * @brief  A typeface / font resource (TTF, OTF, ...).
 *
 * FontResource owns an IDWriteTextFormat or custom font-face once loaded.
 */
class FontResource final : public Resource {
public:
    /**
     * @brief  Construct a font resource stub.
     *
     * @param path  Relative path to the font file.
     * @param id    Unique resource identifier.
     */
    explicit FontResource(std::string path, ID id) noexcept
        : Resource(ResourceType::Font, std::move(path), id)
    {
    }

    /// @brief  TODO:  Load the font via DirectWrite and create a text format.
    [[nodiscard]] bool Load() override;

    /// @brief  TODO:  Release the DirectWrite text format.
    void Unload() noexcept override;
};

} // namespace DragonOS::Resources
