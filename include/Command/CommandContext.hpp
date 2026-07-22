#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::FileSystem { class FileSystemService; }

namespace DragonOS::Command {

class CommandContext final {
public:
    explicit CommandContext(
        FileSystem::FileSystemService& fsService,
        std::wstring currentDirectory,
        std::vector<std::wstring> args)
        : m_fsService{ &fsService }
        , m_currentDirectory{ std::move(currentDirectory) }
        , m_args{ std::move(args) }
    {
    }

    [[nodiscard]] FileSystem::FileSystemService& GetFileSystem() const noexcept
    {
        return *m_fsService;
    }

    [[nodiscard]] const std::wstring& GetCurrentDirectory() const noexcept
    {
        return m_currentDirectory;
    }

    void SetCurrentDirectory(std::wstring dir) noexcept
    {
        m_currentDirectory = std::move(dir);
    }

    [[nodiscard]] const std::vector<std::wstring>& GetArgs() const noexcept
    {
        return m_args;
    }

    [[nodiscard]] size_t GetArgCount() const noexcept { return m_args.size(); }

    [[nodiscard]] const std::wstring& GetArg(size_t index) const noexcept
    {
        static const std::wstring empty;
        return index < m_args.size() ? m_args[index] : empty;
    }

private:
    FileSystem::FileSystemService* m_fsService{ nullptr };
    std::wstring                   m_currentDirectory;
    std::vector<std::wstring>      m_args;
};

} // namespace DragonOS::Command
