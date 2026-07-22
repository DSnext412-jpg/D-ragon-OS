#pragma once

#include <Graphics/Color.hpp>
#include <Theme/ThemeColor.hpp>
#include <Theme/ThemeManager.hpp>

namespace DragonOS::Terminal {

struct TerminalColors final {
    Graphics::Color background{ 0.08f, 0.08f, 0.10f, 1.0f };
    Graphics::Color foreground{ 0.94f, 0.94f, 0.94f, 1.0f };
    Graphics::Color selection{ 0.23f, 0.53f, 1.0f, 0.4f };
    Graphics::Color cursor{ 0.94f, 0.94f, 0.94f, 1.0f };
    Graphics::Color cursorBg{ 0.23f, 0.53f, 1.0f, 0.6f };
    Graphics::Color prompt{ 0.23f, 0.53f, 1.0f, 1.0f };
    Graphics::Color statusLine{ 0.12f, 0.12f, 0.14f, 1.0f };
    Graphics::Color statusText{ 0.60f, 0.60f, 0.60f, 1.0f };
    Graphics::Color scrollbar{ 0.40f, 0.40f, 0.42f, 0.6f };
    Graphics::Color scrollbarHover{ 0.50f, 0.50f, 0.52f, 0.8f };

    static TerminalColors FromTheme(const Theme::ThemeManager& themeMgr) noexcept;
};

} // namespace DragonOS::Terminal
