#pragma once

#include <Engine/System.hpp>
#include <DragonUI/Core/WindowHost.hpp>
#include <memory>

namespace DragonOS::DragonUI {

class DragonUISystem final : public Engine::System {
public:
    DragonUISystem() noexcept = default;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    [[nodiscard]] WindowHost& GetHost() noexcept { return *m_host; }
    [[nodiscard]] const WindowHost& GetHost() const noexcept { return *m_host; }

private:
    std::unique_ptr<WindowHost> m_host;
    Input::InputManager* m_input{};
};

} // namespace
