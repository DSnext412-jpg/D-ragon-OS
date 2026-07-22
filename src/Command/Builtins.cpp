#include <Command/Builtins.hpp>
#include <Command/Command.hpp>
#include <Command/CommandContext.hpp>
#include <Command/CommandResult.hpp>

#include <FileSystem/FileSystemService.hpp>
#include <FileSystem/FileEntry.hpp>

#include <Windows.h>

#undef GetCurrentDirectory
#undef SetCurrentDirectory

#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace DragonOS::Command {
namespace {

// ── Help command ─────────────────────────────────────────────────────────

class HelpCommand final : public Command {
public:
    HelpCommand(CommandRegistry& registry) noexcept
        : m_registry{ &registry }
    {
    }

    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"help";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display available commands or help for a specific command.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help =
            L"help [command]\n"
            L"  Display all available commands, or detailed help for a specific command.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() > 0)
        {
            const auto& cmdName = context.GetArg(0);
            const auto* cmd = m_registry->Find(cmdName);
            if (!cmd)
            {
                result.status = CommandStatus::NotFound;
                result.error = L"No help available for '" + cmdName + L"'";
                return result;
            }

            result.output = cmd->GetName() + L" - " + cmd->GetDescription() + L"\n";
            result.output += cmd->GetHelp();
        }
        else
        {
            result.output = L"Available commands:\n";
            const auto all = m_registry->GetAll();
            for (const auto* cmd : all)
            {
                result.output += L"  " + cmd->GetName();
                size_t pad = cmd->GetName().size() < 16 ? 16 - cmd->GetName().size() : 2;
                result.output += std::wstring(pad, L' ');
                result.output += cmd->GetDescription() + L"\n";
            }
        }

        return result;
    }

private:
    CommandRegistry* m_registry{ nullptr };
};

// ── Clear command ────────────────────────────────────────────────────────

class ClearCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"clear";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Clear the terminal screen.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"clear\n  Clear all output from the terminal.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;
        result.output = L"\x1b[2J";
        return result;
    }
};

// ── Cls command (alias for clear) ────────────────────────────────────────

class ClsCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"cls";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Clear the terminal screen (alias for clear).";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"cls\n  Clear all output from the terminal.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;
        result.output = L"\x1b[2J";
        return result;
    }
};

// ── Version command ──────────────────────────────────────────────────────

class VersionCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"version";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display DragonOS version information.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"version\n  Display the current DragonOS version.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;
        result.output = L"DragonOS Terminal v0.1.0\n"
                        L"Copyright (c) 2026 The DragonOS Project";
        return result;
    }
};

// ── Echo command ─────────────────────────────────────────────────────────

class EchoCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"echo";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display a line of text.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"echo [text...]\n  Print the given text to the terminal.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;
        for (size_t i = 0; i < context.GetArgCount(); ++i)
        {
            if (i > 0) { result.output += L' '; }
            result.output += context.GetArg(i);
        }
        return result;
    }
};

// ── Pwd command ──────────────────────────────────────────────────────────

class PwdCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"pwd";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Print the current working directory.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"pwd\n  Display the current working directory path.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;
        result.output = context.GetCurrentDirectory();
        if (result.output.empty())
        {
            result.output = L"C:\\";
        }
        return result;
    }
};

// ── Ls command ───────────────────────────────────────────────────────────

class LsCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"ls";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"List directory contents.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"ls [path]\n  List files and directories in the given path or current directory.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        std::wstring path = context.GetCurrentDirectory();
        if (context.GetArgCount() > 0)
        {
            path = context.GetArg(0);
        }

        if (path.empty()) { path = L"C:\\"; }

        auto& fs = context.GetFileSystem();
        auto dirResult = fs.ListDirectory(path);

        if (!dirResult.success || dirResult.entries.empty())
        {
            auto drives = fs.GetLogicalDrives();
            for (const auto& drive : drives)
            {
                result.output += drive + L"\\\n";
            }
            return result;
        }

        size_t maxNameLen = 0;
        for (const auto& entry : dirResult.entries)
        {
            maxNameLen = (std::max)(maxNameLen, entry.name.size());
        }
        maxNameLen = (std::min)(maxNameLen, size_t(48));
        maxNameLen = (std::max)(maxNameLen, size_t(16));

        for (const auto& entry : dirResult.entries)
        {
            if (entry.IsDirectory())
            {
                result.output += L"[DIR]  ";
            }
            else
            {
                result.output += L"       ";
            }

            auto name = entry.name;
            if (name.size() > maxNameLen)
            {
                name = name.substr(0, maxNameLen - 3) + L"...";
            }
            result.output += name;
            result.output += std::wstring(maxNameLen - name.size(), L' ');
            result.output += L"\n";
        }

        return result;
    }
};

// ── Dir command (alias for ls) ───────────────────────────────────────────

class DirCommand final : public Command {
public:
    DirCommand(CommandRegistry& registry) noexcept
        : m_registry{ &registry }
    {
    }

    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"dir";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"List directory contents (alias for ls).";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"dir [path]\n  List files and directories in the given path or current directory.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        auto* ls = m_registry->Find(L"ls");
        if (ls)
        {
            return ls->Execute(context);
        }
        CommandResult result;
        result.status = CommandStatus::InternalError;
        result.error = L"Internal error: ls not available";
        return result;
    }

private:
    CommandRegistry* m_registry{ nullptr };
};

// ── Cd command ───────────────────────────────────────────────────────────

class CdCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"cd";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Change the current directory.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"cd <path>\n  Change the current working directory to the given path.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() == 0)
        {
            result.status = CommandStatus::InvalidArgument;
            result.error = L"cd: missing argument";
            return result;
        }

        std::wstring path = context.GetArg(0);

        auto& fs = context.GetFileSystem();

        auto dirResult = fs.ListDirectory(path);
        if (!dirResult.success)
        {
            auto drives = fs.GetLogicalDrives();
            bool isDrive = false;
            for (const auto& drive : drives)
            {
                if (path == drive || path == drive + L"\\")
                {
                    isDrive = true;
                    break;
                }
            }
            if (!isDrive && !fs.Exists(path))
            {
                result.status = CommandStatus::Failure;
                result.error = L"cd: directory not found: " + path;
                return result;
            }
        }

        context.SetCurrentDirectory(path);
        result.output = L"";
        return result;
    }
};

// ── Mkdir command ────────────────────────────────────────────────────────

class MkdirCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"mkdir";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Create a new directory.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"mkdir <name>\n  Create a new directory with the given name.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() == 0)
        {
            result.status = CommandStatus::InvalidArgument;
            result.error = L"mkdir: missing directory name";
            return result;
        }

        const auto& dirName = context.GetArg(0);

        if (::CreateDirectoryW(dirName.c_str(), nullptr))
        {
            result.output = L"Directory created: " + dirName;
        }
        else
        {
            result.status = CommandStatus::Failure;
            result.error = L"mkdir: failed to create directory";
        }

        return result;
    }
};

// ── Rmdir command ────────────────────────────────────────────────────────

class RmdirCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"rmdir";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Remove an empty directory.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"rmdir <name>\n  Remove the specified empty directory.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() == 0)
        {
            result.status = CommandStatus::InvalidArgument;
            result.error = L"rmdir: missing directory name";
            return result;
        }

        const auto& dirName = context.GetArg(0);

        if (::RemoveDirectoryW(dirName.c_str()))
        {
            result.output = L"Directory removed: " + dirName;
        }
        else
        {
            result.status = CommandStatus::Failure;
            result.error = L"rmdir: failed to remove directory";
        }

        return result;
    }
};

// ── Touch command ────────────────────────────────────────────────────────

class TouchCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"touch";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Create an empty file or update its timestamp.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"touch <filename>\n  Create an empty file or update its last-modified timestamp.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() == 0)
        {
            result.status = CommandStatus::InvalidArgument;
            result.error = L"touch: missing filename";
            return result;
        }

        const auto& filename = context.GetArg(0);

        HANDLE hFile = ::CreateFileW(
            filename.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(hFile);
            result.output = L"Touched: " + filename;
        }
        else
        {
            result.status = CommandStatus::Failure;
            result.error = L"touch: failed to create file: " + filename;
        }

        return result;
    }
};

// ── Cat command ──────────────────────────────────────────────────────────

class CatCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"cat";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display the contents of a file.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"cat <filename>\n  Read and display the contents of a text file.";
        return help;
    }

    CommandResult Execute(CommandContext& context) override
    {
        CommandResult result;

        if (context.GetArgCount() == 0)
        {
            result.status = CommandStatus::InvalidArgument;
            result.error = L"cat: missing filename";
            return result;
        }

        const auto& filename = context.GetArg(0);

        HANDLE hFile = ::CreateFileW(
            filename.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            result.status = CommandStatus::Failure;
            result.error = L"cat: file not found: " + filename;
            return result;
        }

        LARGE_INTEGER fileSize;
        if (!::GetFileSizeEx(hFile, &fileSize))
        {
            ::CloseHandle(hFile);
            result.status = CommandStatus::Failure;
            result.error = L"cat: failed to read file size";
            return result;
        }

        LARGE_INTEGER maxCatSize;
        maxCatSize.QuadPart = 1048576;
        if (fileSize.QuadPart > maxCatSize.QuadPart)
        {
            ::CloseHandle(hFile);
            result.status = CommandStatus::Failure;
            result.error = L"cat: file too large to display";
            return result;
        }

        const DWORD size = static_cast<DWORD>(fileSize.QuadPart);
        std::string utf8Buffer(size, '\0');
        DWORD bytesRead = 0;

        if (!::ReadFile(hFile, utf8Buffer.data(), size, &bytesRead, nullptr))
        {
            ::CloseHandle(hFile);
            result.status = CommandStatus::Failure;
            result.error = L"cat: failed to read file";
            return result;
        }
        ::CloseHandle(hFile);

        int wideLen = ::MultiByteToWideChar(
            CP_UTF8, 0, utf8Buffer.data(), static_cast<int>(bytesRead),
            nullptr, 0);

        if (wideLen > 0)
        {
            result.output.resize(static_cast<size_t>(wideLen));
            ::MultiByteToWideChar(
                CP_UTF8, 0, utf8Buffer.data(), static_cast<int>(bytesRead),
                &result.output[0], wideLen);
        }

        return result;
    }
};

// ── Exit command ─────────────────────────────────────────────────────────

class ExitCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"exit";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Exit the terminal.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"exit\n  Close the current terminal window.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;
        result.output = L"\x1b[EXIT";
        return result;
    }
};

// ── Time command ─────────────────────────────────────────────────────────

class TimeCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"time";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display the current system time.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"time\n  Print the current system time.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;

        SYSTEMTIME st;
        ::GetLocalTime(&st);

        wchar_t buf[128];
        ::swprintf_s(buf, L"%02hu:%02hu:%02hu", st.wHour, st.wMinute, st.wSecond);
        result.output = buf;

        return result;
    }
};

// ── Date command ─────────────────────────────────────────────────────────

class DateCommand final : public Command {
public:
    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"date";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display the current system date.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"date\n  Print the current system date.";
        return help;
    }

    CommandResult Execute(CommandContext& /*context*/) override
    {
        CommandResult result;

        SYSTEMTIME st;
        ::GetLocalTime(&st);

        wchar_t buf[128];
        ::swprintf_s(buf, L"%02hu/%02hu/%04hu", st.wMonth, st.wDay, st.wYear);
        result.output = buf;

        return result;
    }
};

} // anonymous namespace

// ============================================================================
//  Registration
// ============================================================================

void RegisterBuiltins(CommandRegistry& registry) noexcept
{
    registry.Register(std::make_unique<HelpCommand>(registry));
    registry.Register(std::make_unique<ClearCommand>());
    registry.Register(std::make_unique<ClsCommand>());
    registry.Register(std::make_unique<VersionCommand>());
    registry.Register(std::make_unique<EchoCommand>());
    registry.Register(std::make_unique<PwdCommand>());
    registry.Register(std::make_unique<LsCommand>());
    registry.Register(std::make_unique<DirCommand>(registry));
    registry.Register(std::make_unique<CdCommand>());
    registry.Register(std::make_unique<MkdirCommand>());
    registry.Register(std::make_unique<RmdirCommand>());
    registry.Register(std::make_unique<TouchCommand>());
    registry.Register(std::make_unique<CatCommand>());
    registry.Register(std::make_unique<ExitCommand>());
}

} // namespace DragonOS::Command
