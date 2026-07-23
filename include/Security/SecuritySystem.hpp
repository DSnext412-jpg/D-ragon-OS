#pragma once

#include <Engine/System.hpp>
#include <Security/UserManager.hpp>
#include <Security/PermissionManager.hpp>
#include <Security/CredentialManager.hpp>
#include <Security/SecurityConfig.hpp>
#include <Security/LoginScreen.hpp>

#include <memory>

namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; class MouseManager; }
namespace DragonOS::Command { class CommandRegistry; }

namespace DragonOS::Security {

class SecuritySystem final : public Engine::System {
public:
    SecuritySystem() noexcept = default;
    ~SecuritySystem() noexcept override { Shutdown(); }

    SecuritySystem(const SecuritySystem&) = delete;
    SecuritySystem& operator=(const SecuritySystem&) = delete;
    SecuritySystem(SecuritySystem&&) = delete;
    SecuritySystem& operator=(SecuritySystem&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    UserManager& GetUserManager() noexcept { return *m_pUserMgr; }
    PermissionManager& GetPermissionManager() noexcept { return *m_pPermMgr; }
    CredentialManager& GetCredentialManager() noexcept { return *m_pCredMgr; }
    SecurityConfig& GetSecurityConfig() noexcept { return *m_pSecConfig; }

    void SetThemeManager(Theme::ThemeManager& tm) noexcept { m_pThemeMgr = &tm; }
    void SetInputManager(Input::InputManager& im) noexcept { m_pInputMgr = &im; }
    void SetMouseManager(Input::MouseManager& mm) noexcept { m_pMouseMgr = &mm; }
    void SetCommandRegistry(Command::CommandRegistry& cr) noexcept { m_pCmdReg = &cr; }

private:
    std::unique_ptr<UserManager> m_pUserMgr;
    std::unique_ptr<PermissionManager> m_pPermMgr;
    std::unique_ptr<CredentialManager> m_pCredMgr;
    std::unique_ptr<SecurityConfig> m_pSecConfig;
    std::unique_ptr<LoginScreen> m_pLoginScreen;

    Theme::ThemeManager* m_pThemeMgr{ nullptr };
    Input::InputManager* m_pInputMgr{ nullptr };
    Input::MouseManager* m_pMouseMgr{ nullptr };
    Command::CommandRegistry* m_pCmdReg{ nullptr };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
    bool m_initialized{ false };
};

} // namespace DragonOS::Security
