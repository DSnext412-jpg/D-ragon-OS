#include <Security/PermissionManager.hpp>
#include <Security/UserManager.hpp>

#include <Engine/EngineContext.hpp>

namespace DragonOS::Security {

bool PermissionManager::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    (void)ctx;
    m_initialized = true;
    return true;
}

void PermissionManager::Shutdown() noexcept
{
    if (!m_initialized) return;
    m_overrides.clear();
    m_pUserMgr = nullptr;
    m_initialized = false;
}

void PermissionManager::Update(float /*deltaTime*/) noexcept {}
void PermissionManager::Render(Engine::EngineContext& /*ctx*/) noexcept {}
void PermissionManager::Resize(float /*width*/, float /*height*/) noexcept {}

bool PermissionManager::CheckPermission(Permission required) const noexcept
{
    if (!m_pUserMgr) return false;
    const auto& ctx = m_pUserMgr->GetSecurityContext();
    return HasPermission(ctx.effectivePermissions, required);
}

bool PermissionManager::CheckPermissionForUser(UserId uid, Permission required) const noexcept
{
    if (!m_pUserMgr) return false;
    auto* user = m_pUserMgr->FindUser(uid);
    if (!user) return false;

    auto basePerms = GetPermissionsForRole(user->role);

    auto it = m_overrides.find(uid);
    if (it != m_overrides.end())
    {
        basePerms = static_cast<Permission>(
            (static_cast<uint64_t>(basePerms) | static_cast<uint64_t>(it->second.granted)) &
            ~static_cast<uint64_t>(it->second.denied));
    }

    return HasPermission(basePerms, required);
}

Permission PermissionManager::GetEffectivePermissions() const noexcept
{
    if (!m_pUserMgr) return Permission::None;
    return m_pUserMgr->GetSecurityContext().effectivePermissions;
}

Permission PermissionManager::GetPermissionsForRole(UserRole role) const noexcept
{
    return GetDefaultRolePermissions(role).permissions;
}

bool PermissionManager::GrantPermission(UserId uid, Permission permission) noexcept
{
    m_overrides[uid].granted = m_overrides[uid].granted | permission;
    return true;
}

bool PermissionManager::RevokePermission(UserId uid, Permission permission) noexcept
{
    m_overrides[uid].denied = m_overrides[uid].denied | permission;
    return true;
}

const RolePermissions& PermissionManager::GetRoleInfo(UserRole role) const noexcept
{
    static const auto& cache = []() {
        static std::unordered_map<UserRole, RolePermissions> map;
        map[UserRole::Guest] = GetDefaultRolePermissions(UserRole::Guest);
        map[UserRole::User] = GetDefaultRolePermissions(UserRole::User);
        map[UserRole::Administrator] = GetDefaultRolePermissions(UserRole::Administrator);
        return map;
    }();
    return cache.at(role);
}

} // namespace DragonOS::Security
