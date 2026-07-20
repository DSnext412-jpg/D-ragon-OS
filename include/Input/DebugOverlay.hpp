#pragma once

#include <Engine/System.hpp>

#include <cstdint>
#include <string>

namespace DragonOS::Input { class InputManager; }
namespace DragonOS::WindowManager { class WindowManager; }

namespace DragonOS::Input {

class DebugOverlay final : public Engine::System {
public:
    DebugOverlay() noexcept = default;

    [[nodiscard]] bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float /*width*/, float /*height*/) noexcept override;

    void SetInputManager(InputManager& mgr) noexcept { m_pInput = &mgr; }
    void SetWindowManager(WindowManager::WindowManager& wm) noexcept { m_pWindowManager = &wm; }

private:
    InputManager*                m_pInput{ nullptr };
    WindowManager::WindowManager* m_pWindowManager{ nullptr };

    // FPS counter
    float    m_frameTime{ 0.0f };
    float    m_fpsAccum{ 0.0f };
    uint32_t m_frameCount{ 0 };
    uint32_t m_fps{ 0 };
};

}
