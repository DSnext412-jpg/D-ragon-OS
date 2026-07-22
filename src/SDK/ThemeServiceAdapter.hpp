#pragma once

#include <DragonOS/Theme.hpp>

#include <Theme/ThemeManager.hpp>

namespace DragonOS::SDK {

class ThemeServiceAdapter final : public dragonos::sdk::IThemeService {
public:
    explicit ThemeServiceAdapter(
        Theme::ThemeManager& mgr) noexcept
        : m_themeManager{ mgr }
    {
    }

    dragonos::sdk::ThemeColor GetColor(
        dragonos::sdk::ThemeToken token) const noexcept override;
    bool IsDarkMode() const noexcept override;

private:
    Theme::ThemeManager& m_themeManager;
};

} // namespace DragonOS::SDK
