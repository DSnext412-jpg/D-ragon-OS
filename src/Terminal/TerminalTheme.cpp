#include <Terminal/TerminalTheme.hpp>

#include <Theme/ThemePalette.hpp>

using namespace DragonOS::Theme;

namespace DragonOS::Terminal {

TerminalColors TerminalColors::FromTheme(
    const Theme::ThemeManager& themeMgr) noexcept
{
    TerminalColors c;

    const auto& theme = themeMgr.GetCurrentTheme();
    const auto& palette = theme.GetPalette();

    auto toColor = [](const Theme::ThemeColor& tc) -> Graphics::Color {
        return { tc.r, tc.g, tc.b, tc.a };
    };

    c.background = toColor(palette.Get(Theme::SemanticColor::WindowBackground));
    c.foreground = toColor(palette.Get(Theme::SemanticColor::TextPrimary));
    c.selection  = toColor(palette.Get(Theme::SemanticColor::Selection));
    c.prompt     = toColor(palette.Get(Theme::SemanticColor::Accent));

    return c;
}

} // namespace DragonOS::Terminal
