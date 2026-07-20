/**
 * @file    ResourceType.hpp
 * @brief   Enumeration of all resource types in DragonOS.
 */

#pragma once

namespace DragonOS::Resources {

/**
 * @brief  Identifies the kind of a resource object.
 *
 * Every Resource carries a ResourceType so the ResourceManager can
 * dispatch load / unload / reload operations without RTTI.
 */
enum class ResourceType {
    Unknown,          ///< Uninitialised or unrecognised resource.
    Image,            ///< Bitmap / raster image (loaded via WIC in a later phase).
    Icon,             ///< Window or shell icon.
    Font,             ///< Typeface / font resource.
    Theme,            ///< Visual theme (colours, metrics, styles).
    Configuration,    ///< Application or system configuration data.
    Audio,            ///< Sound effect or music clip.
    Cursor,           ///< Mouse cursor image.
    Shader,           ///< GPU shader program.
    Animation,        ///< Predefined animation sequence.
    Localization,     ///< Localised string table.
};

} // namespace DragonOS::Resources
