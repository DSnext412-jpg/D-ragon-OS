/**
 * @file    Theme.hpp
 * @brief   Aggregates every visual property that defines a named theme.
 *
 * A Theme is a complete set of palette colours, typography, metrics,
 * shadow definitions, and a mode identifier.  Themes are immutable
 * after construction and are accessed through ThemeManager.
 */

#pragma once

#include <Theme/ThemeMode.hpp>
#include <Theme/ThemePalette.hpp>
#include <Theme/ThemeTypography.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Theme/ThemeShadow.hpp>

#include <string_view>

namespace DragonOS::Theme {

/**
 * @brief  A complete, immutable visual theme.
 *
 * Owns:
 *   - Palette    (every semantic colour)
 *   - Typography (font families and sizes)
 *   - Metrics   (constexpr sizing constants)
 *   - Shadow    (drop-shadow definitions)
 *   - Mode      (Dark / Light / System)
 */
class Theme final {
public:
    /**
     * @brief  Construct a theme with all properties.
     *
     * @param name       Display / identifier name (e.g. "DragonOS Dark").
     * @param mode       Colour-scheme mode.
     * @param palette    Semantic colour palette.
     * @param typography Font definitions.
     * @param shadow     Shadow-effect definitions.
     */
    explicit Theme(
        std::string_view    name,
        ThemeMode           mode,
        ThemePalette        palette,
        ThemeTypography     typography,
        ThemeShadow         shadow) noexcept
        : m_name(name)
        , m_mode(mode)
        , m_palette(std::move(palette))
        , m_typography(typography)
        , m_shadow(shadow)
    {
    }

    // ── Accessors ─────────────────────────────────────────────────────────

    /// @brief  The display name of this theme.
    [[nodiscard]] std::string_view       GetName()       const noexcept { return m_name; }

    /// @brief  The colour-scheme mode.
    [[nodiscard]] ThemeMode              GetMode()       const noexcept { return m_mode; }

    /// @brief  The semantic colour palette.
    [[nodiscard]] const ThemePalette&    GetPalette()    const noexcept { return m_palette; }

    /// @brief  Typography definitions.
    [[nodiscard]] const ThemeTypography& GetTypography() const noexcept { return m_typography; }

    /// @brief  Immutable sizing constants.
    [[nodiscard]] ThemeMetrics           GetMetrics()    const noexcept { return ThemeMetrics{}; }

    /// @brief  Shadow-effect definitions.
    [[nodiscard]] const ThemeShadow&     GetShadow()     const noexcept { return m_shadow; }

private:
    std::string_view m_name;
    ThemeMode        m_mode;
    ThemePalette     m_palette;
    ThemeTypography  m_typography;
    ThemeShadow      m_shadow;
};

} // namespace DragonOS::Theme
