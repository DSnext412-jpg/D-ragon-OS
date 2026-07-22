#pragma once

#include <Command/CommandContext.hpp>
#include <Command/CommandResult.hpp>

#include <memory>
#include <string>

namespace DragonOS::Command {

class Command {
public:
    virtual ~Command() noexcept = default;

    [[nodiscard]] virtual const std::wstring& GetName() const noexcept = 0;
    [[nodiscard]] virtual const std::wstring& GetDescription() const noexcept = 0;
    [[nodiscard]] virtual const std::wstring& GetHelp() const noexcept = 0;

    [[nodiscard]] virtual CommandResult Execute(CommandContext& context) = 0;

protected:
    Command() noexcept = default;
};

using CommandPtr = std::unique_ptr<Command>;

} // namespace DragonOS::Command
