#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Security {

using UserId = uint32_t;

inline constexpr UserId InvalidUserId = 0;
inline constexpr UserId BuiltinAdminUserId = 1;
inline constexpr UserId BuiltinGuestUserId = 2;
inline constexpr UserId FirstCustomUserId = 100;

enum class Permission : uint64_t {
    None = 0,
    FileRead = 1ULL << 0,
    FileWrite = 1ULL << 1,
    FileDelete = 1ULL << 2,
    FileExecute = 1ULL << 3,
    AppLaunch = 1ULL << 4,
    AppInstall = 1ULL << 5,
    SettingsRead = 1ULL << 6,
    SettingsWrite = 1ULL << 7,
    PluginLoad = 1ULL << 8,
    PluginConfigure = 1ULL << 9,
    UserManage = 1ULL << 10,
    SessionManage = 1ULL << 11,
    SystemShutdown = 1ULL << 12,
    AuditLog = 1ULL << 13,
    All = 0xFFFFFFFFFFFFFFFFULL,
};

inline constexpr Permission operator|(Permission a, Permission b) noexcept
{
    return static_cast<Permission>(
        static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

inline constexpr Permission operator&(Permission a, Permission b) noexcept
{
    return static_cast<Permission>(
        static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

inline constexpr bool HasPermission(Permission flags, Permission check) noexcept
{
    return (static_cast<uint64_t>(flags) & static_cast<uint64_t>(check)) != 0;
}

enum class UserRole : uint8_t {
    Guest = 0,
    User = 1,
    Administrator = 2,
};

struct UserProfile final {
    UserId uid{ InvalidUserId };
    std::wstring username;
    std::wstring displayName;
    UserRole role{ UserRole::User };
    bool isGuest{ false };
    bool isLocked{ false };
    std::wstring homeDirectory;
    uint64_t createdAt{ 0 };
};

struct UserSession final {
    UserId uid{ InvalidUserId };
    std::wstring token;
    uint64_t loginTime{ 0 };
    bool isLocked{ false };
};

struct RolePermissions final {
    UserRole role{ UserRole::User };
    std::wstring name;
    std::wstring description;
    Permission permissions{ Permission::None };
};

struct SecurityContext final {
    UserProfile user;
    UserSession session;
    Permission effectivePermissions{ Permission::None };
};

inline constexpr RolePermissions GetDefaultRolePermissions(UserRole role) noexcept
{
    switch (role)
    {
    case UserRole::Administrator:
        return { role, L"Administrator", L"Full system access", Permission::All };
    case UserRole::User:
        return { role, L"User", L"Standard user",
            Permission::FileRead | Permission::FileWrite |
            Permission::AppLaunch | Permission::SettingsRead |
            Permission::SettingsWrite };
    case UserRole::Guest:
        return { role, L"Guest", L"Limited guest access",
            Permission::FileRead | Permission::AppLaunch |
            Permission::SettingsRead };
    }
    return {};
}

} // namespace DragonOS::Security
