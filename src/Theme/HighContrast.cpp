#include <Theme/HighContrast.hpp>

namespace DragonOS::Theme {

namespace {

ThemePalette CreateHighContrastPalette() noexcept
{
    ThemePalette p;

    // High contrast #1 — black background, near-white text, strong accents.
    constexpr auto RGB = [](std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        return ThemeColor::FromRGB(r, g, b);
    };

    p[SemanticColor::DesktopBackground]       = RGB(0, 0, 0);
    p[SemanticColor::DesktopGradientTop]      = RGB(0, 0, 0);
    p[SemanticColor::DesktopGradientBottom]   = RGB(0, 0, 0);

    p[SemanticColor::WindowBackground]        = RGB(0, 0, 0);
    p[SemanticColor::WindowBorder]            = RGB(255, 255, 255);
    p[SemanticColor::WindowTitle]             = RGB(255, 255, 255);
    p[SemanticColor::WindowTitleBar]          = RGB(0, 0, 0);

    p[SemanticColor::TextPrimary]             = RGB(255, 255, 255);
    p[SemanticColor::TextSecondary]           = RGB(255, 255, 0);

    p[SemanticColor::Accent]                  = RGB(255, 255, 0);
    p[SemanticColor::AccentHover]             = RGB(255, 255, 128);
    p[SemanticColor::AccentPressed]           = RGB(200, 200, 0);

    p[SemanticColor::Selection]               = RGB(255, 255, 0);
    p[SemanticColor::Hover]                   = RGB(64, 64, 64);
    p[SemanticColor::Disabled]                = RGB(96, 96, 96);

    p[SemanticColor::Error]                   = RGB(255, 96, 96);
    p[SemanticColor::Warning]                 = RGB(255, 255, 0);
    p[SemanticColor::Success]                 = RGB(96, 255, 96);
    p[SemanticColor::Transparent]             = ThemeColor::FromFloat(0, 0, 0, 0);

    p[SemanticColor::TaskbarBackground]       = RGB(0, 0, 0);
    p[SemanticColor::TaskbarItemHover]        = RGB(64, 64, 64);
    p[SemanticColor::TaskbarItemActive]       = RGB(255, 255, 0);
    p[SemanticColor::StartButtonBackground]   = RGB(255, 255, 0);
    p[SemanticColor::StartButtonHover]        = RGB(255, 255, 128);
    p[SemanticColor::StartButtonPressed]      = RGB(200, 200, 0);

    p[SemanticColor::StartMenuBackground]     = RGB(0, 0, 0);
    p[SemanticColor::StartMenuItemHover]      = RGB(64, 64, 64);

    p[SemanticColor::ExplorerBackground]       = RGB(0, 0, 0);
    p[SemanticColor::ExplorerItemHover]        = RGB(64, 64, 64);
    p[SemanticColor::ExplorerItemSelected]     = RGB(255, 255, 0);
    p[SemanticColor::ExplorerNavigationPane]   = RGB(16, 16, 16);
    p[SemanticColor::ExplorerToolbarBackground] = RGB(16, 16, 16);

    p[SemanticColor::TerminalBackground]       = RGB(0, 0, 0);
    p[SemanticColor::TerminalForeground]       = RGB(255, 255, 255);
    p[SemanticColor::TerminalSelection]        = RGB(255, 255, 0);
    p[SemanticColor::TerminalCursor]           = RGB(255, 255, 0);

    p[SemanticColor::NotificationBackground]   = RGB(0, 0, 0);
    p[SemanticColor::NotificationInfo]         = RGB(255, 255, 0);
    p[SemanticColor::NotificationWarning]      = RGB(255, 255, 0);
    p[SemanticColor::NotificationError]        = RGB(255, 96, 96);
    p[SemanticColor::NotificationSuccess]      = RGB(96, 255, 96);

    p[SemanticColor::SearchBackground]         = RGB(0, 0, 0);
    p[SemanticColor::SearchHighlight]          = RGB(255, 255, 0);

    p[SemanticColor::ServiceIndicator]         = RGB(255, 255, 0);

    p[SemanticColor::ControlFill]              = RGB(0, 0, 0);
    p[SemanticColor::ControlBorder]            = RGB(255, 255, 255);
    p[SemanticColor::ControlText]              = RGB(255, 255, 255);
    p[SemanticColor::PlaceholderText]          = RGB(255, 255, 0);
    p[SemanticColor::CaretColor]               = RGB(255, 255, 255);
    p[SemanticColor::ControlAccentFill]        = RGB(255, 255, 0);
    p[SemanticColor::ControlAccentBorder]      = RGB(255, 255, 0);

    p[SemanticColor::MenuBarBackground]        = RGB(0, 0, 0);
    p[SemanticColor::MenuBackground]           = RGB(0, 0, 0);
    p[SemanticColor::MenuItemHover]            = RGB(64, 64, 64);
    p[SemanticColor::MenuItemSelected]         = RGB(255, 255, 0);
    p[SemanticColor::MenuSeparator]            = RGB(255, 255, 255);
    p[SemanticColor::MenuBarText]              = RGB(255, 255, 255);
    p[SemanticColor::MenuItemText]             = RGB(255, 255, 255);
    p[SemanticColor::MenuItemTextDisabled]     = RGB(96, 96, 96);
    p[SemanticColor::MenuIconGray]             = RGB(255, 255, 0);
    p[SemanticColor::ToolbarBackground]        = RGB(16, 16, 16);
    p[SemanticColor::ToolbarButtonHover]       = RGB(64, 64, 64);
    p[SemanticColor::ToolbarButtonPressed]     = RGB(255, 255, 0);
    p[SemanticColor::StatusBarBackground]      = RGB(0, 0, 0);
    p[SemanticColor::StatusBarText]            = RGB(255, 255, 255);
    p[SemanticColor::StatusBarProgress]        = RGB(255, 255, 0);
    p[SemanticColor::StatusBarProgressTrack]   = RGB(64, 64, 64);

    return p;
}

} // namespace

Theme CreateHighContrastTheme() noexcept
{
    return Theme{
        "DragonOS High Contrast",
        ThemeMode::Dark,
        CreateHighContrastPalette(),
        ThemeTypography{},
        ThemeShadow{}
    };
}

} // namespace DragonOS::Theme