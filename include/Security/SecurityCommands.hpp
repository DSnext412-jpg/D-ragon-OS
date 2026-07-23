#pragma once

namespace DragonOS::Command { class CommandRegistry; }
namespace DragonOS::Security { class UserManager; class PermissionManager; class CredentialManager; }

namespace DragonOS::Security {

void RegisterSecurityCommands(Command::CommandRegistry& registry) noexcept;

void SetSecurityCommandPointers(
    UserManager* um,
    PermissionManager* pm,
    CredentialManager* cm) noexcept;

} // namespace DragonOS::Security
