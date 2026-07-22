#include "ThemeServiceAdapter.hpp"

#include <Theme/ThemePalette.hpp>

namespace DragonOS::SDK {

static Theme::SemanticColor ToSemanticColor(
    dragonos::sdk::ThemeToken token) noexcept
{
    using sdk = dragonos::sdk::ThemeToken;
    using sem = Theme::SemanticColor;

    if (token == sdk::TextPrimary)       return sem::TextPrimary;
    if (token == sdk::TextSecondary)     return sem::TextSecondary;
    if (token == sdk::Accent)            return sem::Accent;
    if (token == sdk::AccentHover)       return sem::AccentHover;
    if (token == sdk::WindowBackground)  return sem::WindowBackground;
    if (token == sdk::WindowBorder)      return sem::WindowBorder;
    if (token == sdk::Error)             return sem::Error;
    if (token == sdk::Warning)           return sem::Warning;
    if (token == sdk::Success)           return sem::Success;
    if (token == sdk::Disabled)          return sem::Disabled;
    return sem::TextPrimary;
}

dragonos::sdk::ThemeColor ThemeServiceAdapter::GetColor(
    dragonos::sdk::ThemeToken token) const noexcept
{
    const auto& internal = m_themeManager.GetColor(ToSemanticColor(token));
    dragonos::sdk::ThemeColor result;
    result.r = internal.r;
    result.g = internal.g;
    result.b = internal.b;
    result.a = internal.a;
    return result;
}

bool ThemeServiceAdapter::IsDarkMode() const noexcept
{
    return m_themeManager.GetCurrentTheme().GetMode() == Theme::ThemeMode::Dark;
}

} // namespace DragonOS::SDK
