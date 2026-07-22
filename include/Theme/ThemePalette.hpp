/**
 * @file    ThemePalette.hpp
 * @brief   Semantic colour palette — the single source of every colour
 *          used in the UI.
 *
 * Future UI code MUST NOT hardcode colours.  Every colour comes from
 * the active palette via ThemeManager::GetColor().
 */

#pragma once

#include <Theme/ThemeColor.hpp>

#include <array>
#include <cstddef>

namespace DragonOS::Theme {

/**
 * @brief  Semantic colour tokens understood by every theme.
 *
 * Extend this enum when new visual elements are introduced.
 * All code uses the semantic name, never a raw RGB value.
 */
enum class SemanticColor {
    DesktopBackground,    ///< Base desktop backdrop.
    DesktopGradientTop,   ///< Top colour of the wallpaper gradient.
    DesktopGradientBottom,///< Bottom colour of the wallpaper gradient.
    WindowBackground,     ///< Client area of a standard window.
    WindowBorder,         ///< Window chrome border.
    WindowTitle,          ///< Title-bar text colour.
    WindowTitleBar,       ///< Title-bar background colour.
    TextPrimary,          ///< Primary body text.
    TextSecondary,        ///< Muted / secondary text.
    Accent,               ///< Interactive accent (buttons, links).
    AccentHover,          ///< Accent in hover state.
    AccentPressed,        ///< Accent in pressed state.
    Selection,            ///< Highlighted / selected item background.
    Hover,                ///< Hover overlay for interactive elements.
    Disabled,             ///< Disabled element fill.
    Error,                ///< Error state / destructive action.
    Warning,              ///< Warning state.
    Success,              ///< Success / positive state.
    Transparent,          ///< Fully transparent (useful as a sentinel).

    // ── Taskbar ─────────────────────────────────────────────────────────
    TaskbarBackground,        ///< Taskbar bar background.
    TaskbarItemHover,         ///< Taskbar item hover overlay.
    TaskbarItemActive,        ///< Active task indicator colour.
    StartButtonBackground,    ///< Start button background.
    StartButtonHover,         ///< Start button hover state.
    StartButtonPressed,       ///< Start button pressed state.

    // ── Start Menu ─────────────────────────────────────────────────────
    StartMenuBackground,      ///< Start menu panel background.
    StartMenuItemHover,       ///< Start menu item hover overlay.

    // ── Explorer ───────────────────────────────────────────────────────
    ExplorerBackground,       ///< Explorer window client background.
    ExplorerItemHover,        ///< Explorer file item hover overlay.
    ExplorerItemSelected,     ///< Explorer file item selection highlight.
    ExplorerNavigationPane,   ///< Explorer navigation pane background.
    ExplorerToolbarBackground,///< Explorer toolbar background.

    // ── Terminal ───────────────────────────────────────────────────────
    TerminalBackground,       ///< Terminal window background.
    TerminalForeground,       ///< Terminal text foreground.
    TerminalSelection,        ///< Terminal selection highlight.
    TerminalCursor,           ///< Terminal cursor color.

    // ── Notifications ──────────────────────────────────────────────────
    NotificationBackground,   ///< Notification popup background.
    NotificationInfo,         ///< Information notification accent.
    NotificationWarning,      ///< Warning notification accent.
    NotificationError,        ///< Error notification accent.
    NotificationSuccess,      ///< Success notification accent.

    // ── Search ─────────────────────────────────────────────────────────
    SearchBackground,         ///< Search panel background.
    SearchHighlight,          ///< Search result highlight.

    // ── Services ───────────────────────────────────────────────────────
    ServiceIndicator,         ///< Background service activity indicator.
};

/// @brief  Number of semantic colour tokens.
inline constexpr std::size_t SemanticColorCount = 44;

/**
 * @brief  Owns every ThemeColor that a theme defines.
 *
 * Access colours by semantic token via Get() or by named member.
 * The Theme holds one Palette for each mode (Dark, Light).
 */
class ThemePalette final {
public:
    ThemePalette() noexcept = default;

    /// @brief  Look up a colour by semantic token.
    /// @return The ThemeColor for the requested token.
    [[nodiscard]] const ThemeColor& Get(SemanticColor token) const noexcept;

    /// @brief  Mutable access for theme construction.
    [[nodiscard]] ThemeColor& operator[](SemanticColor token) noexcept;

    // ── Named accessors (convenience) ────────────────────────────────────

    [[nodiscard]] const ThemeColor& GetDesktopBackground()       const noexcept { return m_colors[0]; }
    [[nodiscard]] const ThemeColor& GetDesktopGradientTop()      const noexcept { return m_colors[1]; }
    [[nodiscard]] const ThemeColor& GetDesktopGradientBottom()   const noexcept { return m_colors[2]; }
    [[nodiscard]] const ThemeColor& GetWindowBackground()        const noexcept { return m_colors[3]; }
    [[nodiscard]] const ThemeColor& GetWindowBorder()            const noexcept { return m_colors[4]; }
    [[nodiscard]] const ThemeColor& GetWindowTitle()             const noexcept { return m_colors[5]; }
    [[nodiscard]] const ThemeColor& GetWindowTitleBar()          const noexcept { return m_colors[6]; }
    [[nodiscard]] const ThemeColor& GetTextPrimary()             const noexcept { return m_colors[7]; }
    [[nodiscard]] const ThemeColor& GetTextSecondary()           const noexcept { return m_colors[8]; }
    [[nodiscard]] const ThemeColor& GetAccent()                  const noexcept { return m_colors[9]; }
    [[nodiscard]] const ThemeColor& GetAccentHover()             const noexcept { return m_colors[10]; }
    [[nodiscard]] const ThemeColor& GetAccentPressed()           const noexcept { return m_colors[11]; }
    [[nodiscard]] const ThemeColor& GetSelection()               const noexcept { return m_colors[12]; }
    [[nodiscard]] const ThemeColor& GetHover()                   const noexcept { return m_colors[13]; }
    [[nodiscard]] const ThemeColor& GetDisabled()                const noexcept { return m_colors[14]; }
    [[nodiscard]] const ThemeColor& GetError()                   const noexcept { return m_colors[15]; }
    [[nodiscard]] const ThemeColor& GetWarning()                 const noexcept { return m_colors[16]; }
    [[nodiscard]] const ThemeColor& GetSuccess()                 const noexcept { return m_colors[17]; }
    [[nodiscard]] const ThemeColor& GetTransparent()             const noexcept { return m_colors[18]; }
    [[nodiscard]] const ThemeColor& GetTaskbarBackground()       const noexcept { return m_colors[19]; }
    [[nodiscard]] const ThemeColor& GetTaskbarItemHover()        const noexcept { return m_colors[20]; }
    [[nodiscard]] const ThemeColor& GetTaskbarItemActive()       const noexcept { return m_colors[21]; }
    [[nodiscard]] const ThemeColor& GetStartButtonBackground()   const noexcept { return m_colors[22]; }
    [[nodiscard]] const ThemeColor& GetStartButtonHover()        const noexcept { return m_colors[23]; }
    [[nodiscard]] const ThemeColor& GetStartButtonPressed()      const noexcept { return m_colors[24]; }
    [[nodiscard]] const ThemeColor& GetStartMenuBackground()     const noexcept { return m_colors[25]; }
    [[nodiscard]] const ThemeColor& GetStartMenuItemHover()      const noexcept { return m_colors[26]; }
    [[nodiscard]] const ThemeColor& GetExplorerBackground()       const noexcept { return m_colors[27]; }
    [[nodiscard]] const ThemeColor& GetExplorerItemHover()        const noexcept { return m_colors[28]; }
    [[nodiscard]] const ThemeColor& GetExplorerItemSelected()     const noexcept { return m_colors[29]; }
    [[nodiscard]] const ThemeColor& GetExplorerNavigationPane()   const noexcept { return m_colors[30]; }
    [[nodiscard]] const ThemeColor& GetExplorerToolbarBackground() const noexcept { return m_colors[31]; }

    [[nodiscard]] const ThemeColor& GetTerminalBackground()       const noexcept { return m_colors[32]; }
    [[nodiscard]] const ThemeColor& GetTerminalForeground()       const noexcept { return m_colors[33]; }
    [[nodiscard]] const ThemeColor& GetTerminalSelection()        const noexcept { return m_colors[34]; }
    [[nodiscard]] const ThemeColor& GetTerminalCursor()           const noexcept { return m_colors[35]; }

    [[nodiscard]] const ThemeColor& GetNotificationBackground()   const noexcept { return m_colors[36]; }
    [[nodiscard]] const ThemeColor& GetNotificationInfo()         const noexcept { return m_colors[37]; }
    [[nodiscard]] const ThemeColor& GetNotificationWarning()      const noexcept { return m_colors[38]; }
    [[nodiscard]] const ThemeColor& GetNotificationError()        const noexcept { return m_colors[39]; }
    [[nodiscard]] const ThemeColor& GetNotificationSuccess()      const noexcept { return m_colors[40]; }
    [[nodiscard]] const ThemeColor& GetSearchBackground()         const noexcept { return m_colors[41]; }
    [[nodiscard]] const ThemeColor& GetSearchHighlight()          const noexcept { return m_colors[42]; }
    [[nodiscard]] const ThemeColor& GetServiceIndicator()         const noexcept { return m_colors[43]; }

private:
    std::array<ThemeColor, SemanticColorCount> m_colors{};
};

// ── inline implementations ──────────────────────────────────────────────

inline const ThemeColor& ThemePalette::Get(SemanticColor token) const noexcept
{
    return m_colors[static_cast<std::size_t>(token)];
}

inline ThemeColor& ThemePalette::operator[](SemanticColor token) noexcept
{
    return m_colors[static_cast<std::size_t>(token)];
}

} // namespace DragonOS::Theme
