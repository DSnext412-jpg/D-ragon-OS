#pragma once

#include <Security/SecurityTypes.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <functional>

namespace DragonOS::Security {

class CredentialManager;

class UserManager final : public Engine::System {
public:
    UserManager() noexcept = default;
    ~UserManager() noexcept override { Shutdown(); }

    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    UserManager(UserManager&&) = delete;
    UserManager& operator=(UserManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    [[nodiscard]] UserId CreateUser(
        std::wstring_view username,
        std::wstring_view displayName,
        std::wstring_view password,
        UserRole role = UserRole::User) noexcept;

    bool DeleteUser(UserId uid) noexcept;
    bool RenameUser(UserId uid, std::wstring_view newUsername) noexcept;

    [[nodiscard]] UserId Authenticate(
        std::wstring_view username,
        std::wstring_view password) noexcept;

    bool SwitchUser(UserId uid) noexcept;
    bool Logout() noexcept;
    bool LockSession() noexcept;
    bool UnlockSession(std::wstring_view password) noexcept;

    bool CreateGuestSession() noexcept;

    [[nodiscard]] UserProfile* FindUser(UserId uid) noexcept;
    [[nodiscard]] UserProfile* FindUserByName(std::wstring_view username) noexcept;
    [[nodiscard]] const UserProfile* GetCurrentUser() const noexcept;
    [[nodiscard]] UserId GetCurrentUserId() const noexcept { return m_currentSession.uid; }
    [[nodiscard]] bool IsLoggedIn() const noexcept { return m_currentSession.uid != InvalidUserId; }
    [[nodiscard]] bool IsSessionLocked() const noexcept { return m_currentSession.isLocked; }
    [[nodiscard]] const UserSession& GetCurrentSession() const noexcept { return m_currentSession; }
    [[nodiscard]] const SecurityContext& GetSecurityContext() const noexcept { return m_securityContext; }

    [[nodiscard]] std::vector<const UserProfile*> GetAllUsers() const noexcept;

    [[nodiscard]] bool IsAutoLoginEnabled() const noexcept { return m_autoLogin; }
    void SetAutoLogin(bool enabled) noexcept { m_autoLogin = enabled; }

    void SetCredentialManager(CredentialManager& cm) noexcept { m_pCredentialMgr = &cm; }

    using SessionEventCallback = std::function<void(UserId uid, std::wstring_view event)>;
    void SetOnSessionEvent(SessionEventCallback cb) noexcept { m_sessionEventCb = std::move(cb); }

    void SetLoginScreenVisible(bool visible) noexcept;
    [[nodiscard]] bool IsLoginScreenVisible() const noexcept { return m_loginScreenVisible; }

    bool SaveUsers() noexcept;
    bool LoadUsers() noexcept;

private:
    UserId GenerateUserId() noexcept;
    std::wstring GenerateSessionToken() noexcept;
    std::wstring GetUsersFilePath() const noexcept;
    void UpdateSecurityContext() noexcept;

    std::unordered_map<UserId, UserProfile> m_users;
    UserSession m_currentSession;
    SecurityContext m_securityContext;

    UserId m_nextUserId{ FirstCustomUserId };
    bool m_autoLogin{ false };
    bool m_loginScreenVisible{ true };
    std::wstring m_lastUsername;

    CredentialManager* m_pCredentialMgr{ nullptr };

    SessionEventCallback m_sessionEventCb;
    bool m_initialized{ false };
};

} // namespace DragonOS::Security
