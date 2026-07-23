#pragma once

#include <Security/SecurityTypes.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Security {

class UserManager;

class PermissionManager final : public Engine::System {
public:
    PermissionManager() noexcept = default;
    ~PermissionManager() noexcept override { Shutdown(); }

    PermissionManager(const PermissionManager&) = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;
    PermissionManager(PermissionManager&&) = delete;
    PermissionManager& operator=(PermissionManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    [[nodiscard]] bool CheckPermission(Permission required) const noexcept;
    [[nodiscard]] bool CheckPermissionForUser(UserId uid, Permission required) const noexcept;

    [[nodiscard]] Permission GetEffectivePermissions() const noexcept;
    [[nodiscard]] Permission GetPermissionsForRole(UserRole role) const noexcept;

    bool GrantPermission(UserId uid, Permission permission) noexcept;
    bool RevokePermission(UserId uid, Permission permission) noexcept;

    void SetUserManager(UserManager& um) noexcept { m_pUserMgr = &um; }

    [[nodiscard]] const RolePermissions& GetRoleInfo(UserRole role) const noexcept;

private:
    struct PermissionOverride {
        Permission granted{ Permission::None };
        Permission denied{ Permission::None };
    };

    std::unordered_map<UserId, PermissionOverride> m_overrides;
    UserManager* m_pUserMgr{ nullptr };
    bool m_initialized{ false };
};

} // namespace DragonOS::Security
