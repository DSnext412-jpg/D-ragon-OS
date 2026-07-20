/**
 * @file    ThemeTypography.hpp
 * @brief   Typography definitions for the UI.
 *
 * Defines font families and sizes used across the shell.
 * Actual font loading is handled by the Resources module in a later phase.
 */

#pragma once

#include <string_view>

namespace DragonOS::Theme {

/**
 * @brief  Describes a single font face used in the theme.
 *
 * The family name is a logical identifier (e.g. "Segoe UI", "Cascadia Code").
 * Actual IDWriteTextFormat creation belongs to the rendering layer.
 */
struct FontFace final {
    std::string_view family;   ///< Font family name.
    float            size;     ///< Font size in DIPs (device-independent pixels).
};

/**
 * @brief  Contains every font face the UI uses.
 *
 * Default font size is used for standard text; heading and caption
 * sizes provide a consistent typographic scale.
 *
 * TODO (later phase):  Font weight, stretch, and locale support.
 */
struct ThemeTypography final {
    FontFace defaultFont{  "Segoe UI",    14.0f }; ///< Standard UI text.
    FontFace headingFont{  "Segoe UI",    24.0f }; ///< Section / dialog headings.
    FontFace captionFont{  "Segoe UI",    11.0f }; ///< Secondary / helper text.
    FontFace terminalFont{ "Cascadia Code", 13.0f }; ///< Monospace / code text.
};

} // namespace DragonOS::Theme
