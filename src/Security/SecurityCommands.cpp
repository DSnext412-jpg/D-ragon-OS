#include <Security/SecurityCommands.hpp>

#include <Command/Command.hpp>
#include <Command/CommandContext.hpp>
#include <Command/CommandRegistry.hpp>
#include <Command/CommandResult.hpp>

#include <Security/UserManager.hpp>
#include <Security/PermissionManager.hpp>
#include <Security/CredentialManager.hpp>

namespace DragonOS::Security {
namespace {

UserManager* g_pUserMgr = nullptr;
PermissionManager* g_pPermMgr = nullptr;
CredentialManager* g_pCredMgr = nullptr;

class WhoamiCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"whoami";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display the current user.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"whoami\n  Show the currently logged-in username and role.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr || !g_pUserMgr->IsLoggedIn())
        {
            result.output = L"No user is currently logged in.";
            return result;
        }

        const auto* user = g_pUserMgr->GetCurrentUser();
        if (!user)
        {
            result.output = L"Unknown user.";
            return result;
        }

        wchar_t buf[256];
        const wchar_t* roleStr = (user->role == UserRole::Administrator) ? L"Administrator"
            : (user->role == UserRole::Guest) ? L"Guest" : L"User";
        swprintf_s(buf, L"%s (%s) - %s", user->username.c_str(), user->displayName.c_str(), roleStr);
        result.output = buf;
        return result;
    }
};

class LoginCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"login";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Sign in as a different user.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"login <username>\n  Sign in as the specified user (will prompt for password).";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext& context) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr)
        {
            result.output = L"User manager not available.";
            return result;
        }

        if (context.GetArgCount() < 1)
        {
            result.output = L"Usage: login <username>";
            return result;
        }

        g_pUserMgr->Logout();
        result.output = L"Login screen activated. Please sign in.";
        return result;
    }
};

class LogoutCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"logout";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Sign out of the current session.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"logout\n  Sign out and return to the login screen.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr)
        {
            result.output = L"User manager not available.";
            return result;
        }

        if (!g_pUserMgr->IsLoggedIn())
        {
            result.output = L"Not currently logged in.";
            return result;
        }

        g_pUserMgr->Logout();
        result.output = L"Signed out successfully.";
        return result;
    }
};

class PasswdCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"passwd";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Change the current user's password.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"passwd\n  Change password for the current user.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr || !g_pCredMgr)
        {
            result.output = L"Security services not available.";
            return result;
        }

        if (!g_pUserMgr->IsLoggedIn())
        {
            result.output = L"Not logged in.";
            return result;
        }

        const auto* user = g_pUserMgr->GetCurrentUser();
        if (!user)
        {
            result.output = L"User not found.";
            return result;
        }

        if (user->role == UserRole::Guest)
        {
            result.output = L"Guest users cannot change passwords.";
            return result;
        }

        result.output = L"Password change requires interactive prompt.\nUse the Settings app to change your password.";
        return result;
    }
};

class UsersCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"users";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"List all system users.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"users\n  Display all registered user accounts.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr)
        {
            result.output = L"User manager not available.";
            return result;
        }

        auto allUsers = g_pUserMgr->GetAllUsers();
        if (allUsers.empty())
        {
            result.output = L"No users registered.";
            return result;
        }

        result.output = L"User accounts:\n";
        for (const auto* user : allUsers)
        {
            const wchar_t* roleStr = (user->role == UserRole::Administrator) ? L"Admin"
                : (user->role == UserRole::Guest) ? L"Guest" : L"User";
            wchar_t buf[256];
            swprintf_s(buf, L"  %-20s %-15s %s\n",
                user->username.c_str(), roleStr,
                user->isLocked ? L"[Locked]" : L"[Active]");
            result.output += buf;
        }

        return result;
    }
};

class PermissionsCommand final : public Command::Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"permissions";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display effective permissions.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"permissions\n  Show the current user's effective permissions.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        if (!g_pUserMgr)
        {
            result.output = L"User manager not available.";
            return result;
        }

        if (!g_pUserMgr->IsLoggedIn())
        {
            result.output = L"Not logged in.";
            return result;
        }

        const auto& secCtx = g_pUserMgr->GetSecurityContext();
        auto perm = secCtx.effectivePermissions;

        result.output = L"Effective permissions:\n";

        struct PermInfo { const wchar_t* name; Permission flag; };
        const PermInfo permInfos[] = {
            { L"File Read",       Permission::FileRead },
            { L"File Write",      Permission::FileWrite },
            { L"File Delete",     Permission::FileDelete },
            { L"File Execute",    Permission::FileExecute },
            { L"App Launch",      Permission::AppLaunch },
            { L"App Install",     Permission::AppInstall },
            { L"Settings Read",   Permission::SettingsRead },
            { L"Settings Write",  Permission::SettingsWrite },
            { L"Plugin Load",     Permission::PluginLoad },
            { L"Plugin Config",   Permission::PluginConfigure },
            { L"User Manage",     Permission::UserManage },
            { L"Session Manage",  Permission::SessionManage },
            { L"Shutdown",        Permission::SystemShutdown },
            { L"Audit Log",       Permission::AuditLog },
        };

        for (const auto& pi : permInfos)
        {
            wchar_t buf[128];
            swprintf_s(buf, L"  %-18s %s\n",
                pi.name,
                HasPermission(perm, pi.flag) ? L"Granted" : L"Denied");
            result.output += buf;
        }

        return result;
    }
};

} // anonymous namespace

void RegisterSecurityCommands(::DragonOS::Command::CommandRegistry& registry) noexcept
{
    registry.Register(std::make_unique<WhoamiCommand>());
    registry.Register(std::make_unique<LoginCommand>());
    registry.Register(std::make_unique<LogoutCommand>());
    registry.Register(std::make_unique<PasswdCommand>());
    registry.Register(std::make_unique<UsersCommand>());
    registry.Register(std::make_unique<PermissionsCommand>());
}

void SetSecurityCommandPointers(
    UserManager* um,
    PermissionManager* pm,
    CredentialManager* cm) noexcept
{
    g_pUserMgr = um;
    g_pPermMgr = pm;
    g_pCredMgr = cm;
}

} // namespace DragonOS::Security
