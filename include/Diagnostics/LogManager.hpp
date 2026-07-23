#pragma once

#include <Diagnostics/DiagnosticsTypes.hpp>

#include <Engine/System.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Diagnostics {

class LogManager final : public Engine::System {
public:
    LogManager() noexcept = default;
    ~LogManager() noexcept override { Shutdown(); }

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    LogManager(LogManager&&) = delete;
    LogManager& operator=(LogManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    uint64_t Log(
        LogLevel level,
        std::wstring_view source,
        std::wstring_view message,
        std::wstring_view category = L"") noexcept;

    uint64_t Trace(std::wstring_view source, std::wstring_view message) noexcept;
    uint64_t Debug(std::wstring_view source, std::wstring_view message) noexcept;
    uint64_t Info(std::wstring_view source, std::wstring_view message) noexcept;
    uint64_t Warning(std::wstring_view source, std::wstring_view message) noexcept;
    uint64_t Error(std::wstring_view source, std::wstring_view message) noexcept;
    uint64_t Fatal(std::wstring_view source, std::wstring_view message) noexcept;

    std::vector<const LogEntry*> Query(
        LogLevel minLevel = LogLevel::Trace,
        std::wstring_view sourceFilter = L"",
        size_t maxResults = 100) const noexcept;

    std::vector<const LogEntry*> GetRecent(size_t count = 50) const noexcept;
    std::vector<const LogEntry*> GetByLevel(LogLevel level, size_t maxResults = 100) const noexcept;

    size_t GetEntryCount() const noexcept { return m_count; }
    size_t GetUnreadCount() const noexcept { return m_unreadCount; }
    void MarkAllRead() noexcept;

    void SetOnLogCallback(LogCallback cb) noexcept { m_onLog = std::move(cb); }

    void Dismiss(uint64_t entryId) noexcept;
    void Clear() noexcept;

private:
    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    size_t NextIndex() noexcept;

    std::array<LogEntry, MaxLogEntries> m_entries{};
    size_t m_head{ 0 };
    size_t m_count{ 0 };
    size_t m_unreadCount{ 0 };
    LogCallback m_onLog;
    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
