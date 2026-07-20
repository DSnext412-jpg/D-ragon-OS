/**
 * @file    Theme.cpp
 * @brief   Built-in DragonOS Dark Theme definition.
 *
 * The default theme is created in code so the shell always has
 * a valid visual identity, even without parsed theme files.
 */

#include <Theme/Theme.hpp>

namespace DragonOS::Theme {

/**
 * @brief  Construct the built-in DragonOS Dark palette.
 *
 * Colours are chosen for a modern, low-eyestrain dark desktop
 * experience.  All values in 8-bit sRGB, converted to [0, 1] float.
 */
static ThemePalette CreateDarkPalette() noexcept
{
    ThemePalette p;

    // ── Desktop ──────────────────────────────────────────────────────────
    p[SemanticColor::DesktopBackground]      = ThemeColor::FromRGB( 7, 10, 18);    // Near-black
    p[SemanticColor::DesktopGradientTop]     = ThemeColor::FromRGB(20, 24, 34);    // Deep navy
    p[SemanticColor::DesktopGradientBottom]  = ThemeColor::FromRGB( 7, 10, 18);    // Near-black

    // ── Window chrome ─────────────────────────────────────────────────────
    p[SemanticColor::WindowBackground]       = ThemeColor::FromRGB(32, 36, 46);    // Dark slate
    p[SemanticColor::WindowBorder]           = ThemeColor::FromRGB(70, 80, 100);   // Lighter slate
    p[SemanticColor::WindowTitle]            = ThemeColor::FromFloat(1.0f, 1.0f, 1.0f); // Pure white
    p[SemanticColor::WindowTitleBar]         = ThemeColor::FromRGB(24, 28, 38);    // Slightly darker

    // ── Text ──────────────────────────────────────────────────────────────
    p[SemanticColor::TextPrimary]            = ThemeColor::FromRGB(240, 240, 240); // Off-white
    p[SemanticColor::TextSecondary]          = ThemeColor::FromRGB(180, 180, 180); // Grey

    // ── Accent ────────────────────────────────────────────────────────────
    p[SemanticColor::Accent]                 = ThemeColor::FromRGB( 58, 134, 255); // Vivid blue
    p[SemanticColor::AccentHover]            = ThemeColor::FromRGB( 96, 160, 255); // Lighter blue
    p[SemanticColor::AccentPressed]           = ThemeColor::FromRGB( 30, 100, 220); // Darker blue

    // ── Interactive states ────────────────────────────────────────────────
    p[SemanticColor::Selection]              = ThemeColor::FromRGB( 58, 134, 255, 60);  // Blue tint
    p[SemanticColor::Hover]                  = ThemeColor::FromRGB(255, 255, 255, 16);  // Subtle white
    p[SemanticColor::Disabled]               = ThemeColor::FromRGB(80, 84, 94);         // Muted grey

    // ── Semantic states ───────────────────────────────────────────────────
    p[SemanticColor::Error]                  = ThemeColor::FromRGB(220, 50, 50);        // Red
    p[SemanticColor::Warning]                = ThemeColor::FromRGB(230, 170, 40);       // Amber
    p[SemanticColor::Success]                = ThemeColor::FromRGB(50, 190, 80);        // Green

    // ── Utility ───────────────────────────────────────────────────────────
    p[SemanticColor::Transparent]            = ThemeColor::FromFloat(0.0f, 0.0f, 0.0f, 0.0f);

    return p;
}

/**
 * @brief  Construct the built-in DragonOS Dark theme.
 *
 * This is the default theme used until a custom theme is loaded.
 */
Theme CreateDefaultDarkTheme() noexcept
{
    return Theme{
        "DragonOS Dark",
        ThemeMode::Dark,
        CreateDarkPalette(),
        ThemeTypography{},
        ThemeShadow{}
    };
}

} // namespace DragonOS::Theme
