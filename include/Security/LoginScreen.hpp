#pragma once

#include <Security/SecurityTypes.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class MouseManager; class InputManager; }

namespace DragonOS::Security {

class UserManager;

class LoginScreen final {
public:
    LoginScreen() noexcept = default;
    ~LoginScreen() noexcept = default;

    LoginScreen(const LoginScreen&) = delete;
    LoginScreen& operator=(const LoginScreen&) = delete;
    LoginScreen(LoginScreen&&) = delete;
    LoginScreen& operator=(LoginScreen&&) = delete;

    void SetDependencies(
        Theme::ThemeManager& themeManager,
        Input::MouseManager& mouseManager,
        Input::InputManager& inputManager) noexcept;

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;

    void SetUserManager(UserManager& um) noexcept { m_pUserMgr = &um; }

    void SetViewportSize(float width, float height) noexcept
    {
        m_viewportWidth = width;
        m_viewportHeight = height;
    }

    [[nodiscard]] bool WantsInput() const noexcept { return m_visible; }
    void SetVisible(bool visible) noexcept;
    [[nodiscard]] bool IsVisible() const noexcept { return m_visible; }

    void SetOnLoginCallback(std::function<void(UserId)> cb) noexcept
    {
        m_onLogin = std::move(cb);
    }

    void SetOnLockCallback(std::function<void()> cb) noexcept
    {
        m_onLock = std::move(cb);
    }

private:
    void ProcessInput() noexcept;

    void RenderBackground(Graphics::Renderer& renderer) noexcept;
    void RenderLoginBox(Graphics::Renderer& renderer) noexcept;
    void RenderLockBox(Graphics::Renderer& renderer) noexcept;
    void RenderUserList(Graphics::Renderer& renderer) noexcept;
    void RenderUserButton(Graphics::Renderer& renderer, const UserProfile& user, float x, float y, float w, float h, bool hovered) noexcept;

    enum class Mode { Login, Lock, UserSelect };

    Mode m_mode{ Mode::Login };
    bool m_visible{ true };

    std::wstring m_username;
    std::wstring m_password;
    bool m_passwordVisible{ false };
    int m_cursorPos{ 0 };
    bool m_error{ false };
    std::wstring m_errorMessage;

    float m_viewportWidth{ 1280.0f };
    float m_viewportHeight{ 720.0f };

    std::vector<UserProfile> m_userList;
    int m_hoveredUserIdx{ -1 };

    Theme::ThemeManager*  m_pTheme{ nullptr };
    Input::MouseManager*  m_pMouse{ nullptr };
    Input::InputManager*  m_pInput{ nullptr };
    UserManager*          m_pUserMgr{ nullptr };

    std::function<void(UserId)> m_onLogin;
    std::function<void()>       m_onLock;

    bool m_initialized{ false };
};

} // namespace DragonOS::Security
