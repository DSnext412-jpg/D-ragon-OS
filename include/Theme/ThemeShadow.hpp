/**
 * @file    ThemeShadow.hpp
 * @brief   Shadow-effect definitions for UI elements.
 *
 * Describes drop-shadow parameters for windows, menus, popups, and
 * tooltips.  Actual Direct2D shadow rendering is implemented in a
 * later phase.
 */

#pragma once

#include <Theme/ThemeColor.hpp>

namespace DragonOS::Theme {

/**
 * @brief  Parameters for a single drop-shadow effect.
 *
 * Shadows are defined by a colour, offset, blur radius, and opacity.
 */
struct ShadowDef final {
    ThemeColor color;     ///< Shadow tint colour.
    float      offsetX;   ///< Horizontal offset in DIPs.
    float      offsetY;   ///< Vertical offset in DIPs.
    float      blurRadius;///< Gaussian-blur radius in DIPs.
};

/**
 * @brief  Shadow definitions for every elevated surface in the UI.
 *
 * TODO:  Replace default values with theme-driven values once
 *        theme parsing (JSON / custom format) is implemented.
 */
struct ThemeShadow final {
    ShadowDef windowShadow{  { 0.0f, 0.0f, 0.0f, 0.35f }, 0.0f,  8.0f, 16.0f }; ///< Top-level window shadow.
    ShadowDef popupShadow{   { 0.0f, 0.0f, 0.0f, 0.4f }, 0.0f,  4.0f,  8.0f }; ///< Context menu / popup.
    ShadowDef menuShadow{    { 0.0f, 0.0f, 0.0f, 0.4f }, 0.0f,  4.0f,  8.0f }; ///< Drop-down menu.
    ShadowDef tooltipShadow{ { 0.0f, 0.0f, 0.0f, 0.3f }, 0.0f,  2.0f,  4.0f }; ///< Tooltip.
};

} // namespace DragonOS::Theme
