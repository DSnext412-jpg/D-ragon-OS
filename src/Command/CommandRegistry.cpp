#include <Command/CommandRegistry.hpp>

#include <algorithm>
#include <sstream>

namespace DragonOS::Command {

void CommandRegistry::Register(CommandPtr command) noexcept
{
    if (command)
    {
        m_commands[std::wstring{ command->GetName() }] = std::move(command);
    }
}

void CommandRegistry::Unregister(std::wstring_view name) noexcept
{
    m_commands.erase(std::wstring{ name });
}

Command* CommandRegistry::Find(std::wstring_view name) noexcept
{
    auto it = m_commands.find(std::wstring{ name });
    return (it != m_commands.end()) ? it->second.get() : nullptr;
}

const Command* CommandRegistry::Find(std::wstring_view name) const noexcept
{
    auto it = m_commands.find(std::wstring{ name });
    return (it != m_commands.end()) ? it->second.get() : nullptr;
}

std::vector<const Command*> CommandRegistry::GetAll() const noexcept
{
    std::vector<const Command*> result;
    result.reserve(m_commands.size());
    for (const auto& [name, cmd] : m_commands)
    {
        result.push_back(cmd.get());
    }
    return result;
}

CommandResult CommandRegistry::Execute(
    std::wstring_view commandLine,
    CommandContext& context)
{
    CommandResult result;

    if (commandLine.empty())
    {
        result.status = CommandStatus::Success;
        return result;
    }

    std::wstring line{ commandLine };

    while (!line.empty() && (line.front() == L' ' || line.front() == L'\t'))
    {
        line.erase(line.begin());
    }

    if (line.empty())
    {
        result.status = CommandStatus::Success;
        return result;
    }

    std::vector<std::wstring> args;
    std::wstring current;
    bool inQuotes = false;

    for (wchar_t ch : line)
    {
        if (ch == L'"')
        {
            inQuotes = !inQuotes;
        }
        else if (ch == L' ' && !inQuotes)
        {
            if (!current.empty())
            {
                args.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += ch;
        }
    }
    if (!current.empty())
    {
        args.push_back(current);
    }

    if (args.empty())
    {
        result.status = CommandStatus::Success;
        return result;
    }

    const std::wstring& cmdName = args[0];

    auto* cmd = Find(cmdName);
    if (!cmd)
    {
        result.status = CommandStatus::NotFound;
        result.error = L"Command not found: " + cmdName;
        return result;
    }

    std::vector<std::wstring> cmdArgs;
    if (args.size() > 1)
    {
        cmdArgs.assign(args.begin() + 1, args.end());
    }

    context = CommandContext{
        context.GetFileSystem(),
        context.GetCurrentDirectory(),
        std::move(cmdArgs)
    };

    return cmd->Execute(context);
}

} // namespace DragonOS::Command
