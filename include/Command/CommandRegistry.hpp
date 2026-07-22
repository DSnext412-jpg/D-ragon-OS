#pragma once

#include <Command/Command.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Command {

class CommandRegistry final {
public:
    CommandRegistry() noexcept = default;
    ~CommandRegistry() noexcept = default;

    CommandRegistry(const CommandRegistry&) = delete;
    CommandRegistry& operator=(const CommandRegistry&) = delete;
    CommandRegistry(CommandRegistry&&) = delete;
    CommandRegistry& operator=(CommandRegistry&&) = delete;

    void Register(CommandPtr command) noexcept;
    void Unregister(std::wstring_view name) noexcept;

    Command* Find(std::wstring_view name) noexcept;
    const Command* Find(std::wstring_view name) const noexcept;

    std::vector<const Command*> GetAll() const noexcept;

    CommandResult Execute(
        std::wstring_view commandLine,
        CommandContext& context);

    bool IsRegistered(std::wstring_view name) const noexcept
    {
        return m_commands.find(name.data()) != m_commands.end();
    }

    [[nodiscard]] size_t GetCount() const noexcept { return m_commands.size(); }

private:
    std::unordered_map<std::wstring, CommandPtr> m_commands;
};

} // namespace DragonOS::Command
