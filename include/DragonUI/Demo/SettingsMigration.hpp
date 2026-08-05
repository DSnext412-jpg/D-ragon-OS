#pragma once

#include <DragonUI/Core/WindowHost.hpp>
#include <memory>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; }

namespace DragonOS::DragonUI::Demo {

class SettingsMigrationSection final {
public:
    SettingsMigrationSection(
        Graphics::Renderer& renderer,
        Theme::ThemeManager& theme,
        Input::InputManager& input) noexcept;

    void Resize(float width, float height) noexcept;
    void Render() noexcept;
    void Update(float deltaTime) noexcept;

    [[nodiscard]] WindowHost& GetHost() noexcept { return *m_host; }

private:
    std::unique_ptr<WindowHost> m_host;
};

} // namespace
