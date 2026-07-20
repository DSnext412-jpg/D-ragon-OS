/**
 * @file    ThemeMetrics.hpp
 * @brief   Constexpr sizing and spacing constants for the UI.
 *
 * Every dimension used by the shell originates here.
 * No magic numbers anywhere in the UI code.
 */

#pragma once

namespace DragonOS::Theme {

/**
 * @brief  Immutable measurement constants shared by all themes.
 *
 * Values are in DIPs (device-independent pixels) unless noted.
 * A future phase may allow per-theme overrides.
 */
struct ThemeMetrics final {
    /// @brief  Corner radius for standard windows.
    static constexpr float WindowCornerRadius   = 10.0f;

    /// @brief  Thickness of the window chrome border.
    static constexpr float WindowBorderThickness = 1.0f;

    /// @brief  Height of the title bar.
    static constexpr float TitleBarHeight        = 32.0f;

    /// @brief  Height of the taskbar.
    static constexpr float TaskbarHeight         = 48.0f;

    /// @brief  Padding between desktop edge and icons.
    static constexpr float DesktopPadding        = 16.0f;

    /// @brief  Default margin around panels and dialogs.
    static constexpr float DefaultMargin         = 12.0f;

    /// @brief  Default spacing between elements.
    static constexpr float DefaultSpacing        = 8.0f;

    /// @brief  Standard button height.
    static constexpr float ButtonHeight          = 32.0f;

    /// @brief  Small icon size (e.g. status indicators).
    static constexpr float IconSizeSmall         = 16.0f;

    /// @brief  Medium icon size (e.g. desktop icons).
    static constexpr float IconSizeMedium        = 32.0f;

    /// @brief  Large icon size (e.g. taskbar app buttons).
    static constexpr float IconSizeLarge         = 48.0f;

    /// @brief  Drop-shadow offset behind windows.
    static constexpr float WindowShadowOffset    = 6.0f;

    /// @brief  Horizontal padding of title-bar text from window edge.
    static constexpr float TitleTextPaddingX     = 14.0f;

    /// @brief  Vertical padding of title-bar text from window top edge.
    static constexpr float TitleTextPaddingY     = 6.0f;
};

} // namespace DragonOS::Theme
