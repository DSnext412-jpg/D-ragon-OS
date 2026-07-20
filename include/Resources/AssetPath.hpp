/**
 * @file    AssetPath.hpp
 * @brief   Constexpr asset directory paths for DragonOS.
 */

#pragma once

#include <string_view>

namespace DragonOS::Resources {

/**
 * @brief  Central registry of asset directory paths.
 *
 * All paths are relative to the application root and are defined as
 * constexpr std::string_view values for compile-time resolution.
 *
 * Layout:
 *   assets/
 *   assets/icons/
 *   assets/fonts/
 *   assets/themes/
 *   assets/images/
 *   assets/wallpapers/
 */
struct AssetPath final {
    AssetPath() = delete;

    /// @brief  Root asset directory.
    static constexpr std::string_view Root{ "assets/" };

    /// @brief  Application icons.
    static constexpr std::string_view Icons{ "assets/icons/" };

    /// @brief  Font files (e.g. .ttf, .otf).
    static constexpr std::string_view Fonts{ "assets/fonts/" };

    /// @brief  Theme definition files.
    static constexpr std::string_view Themes{ "assets/themes/" };

    /// @brief  Raster images (e.g. .png, .jpg).
    static constexpr std::string_view Images{ "assets/images/" };

    /// @brief  Desktop wallpaper images.
    static constexpr std::string_view Wallpapers{ "assets/wallpapers/" };
};

} // namespace DragonOS::Resources
