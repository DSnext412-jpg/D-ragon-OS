#pragma once

#include <cstdint>
#include <string_view>

namespace dragonos::sdk {

struct ThemeColor {
    float r{ 0.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f };
};

enum class ThemeToken {
    TextPrimary,
    TextSecondary,
    Accent,
    AccentHover,
    WindowBackground,
    WindowBorder,
    Error,
    Warning,
    Success,
    Disabled,
};

class IThemeService {
public:
    virtual ~IThemeService() noexcept = default;
    virtual ThemeColor GetColor(ThemeToken token) const noexcept = 0;
    virtual bool IsDarkMode() const noexcept = 0;
};

} // namespace dragonos::sdk
