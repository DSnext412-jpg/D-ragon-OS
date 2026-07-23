#include <Diagnostics/LogManager.hpp>

namespace DragonOS::Diagnostics {

bool LogManager::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    m_head = 0;
    m_count = 0;
    m_unreadCount = 0;
    m_initialized = true;
    return true;
}

void LogManager::Shutdown() noexcept
{
    if (!m_initialized) return;
    m_onLog = nullptr;
    m_initialized = false;
}

void LogManager::Update(float /*deltaTime*/) noexcept
{
}

void LogManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void LogManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

size_t LogManager::NextIndex() noexcept
{
    size_t idx = m_head;
    if (m_count < MaxLogEntries)
    {
        m_head = (m_head + 1) % MaxLogEntries;
        m_count++;
    }
    else
    {
        m_head = (m_head + 1) % MaxLogEntries;
    }
    return idx;
}

uint64_t LogManager::Log(
    LogLevel level,
    std::wstring_view source,
    std::wstring_view message,
    std::wstring_view category) noexcept
{
    if (!m_initialized) return 0;

    size_t idx = NextIndex();
    auto& entry = m_entries[idx];
    entry.id = NextId();
    entry.level = level;
    entry.source = source;
    entry.message = message;
    entry.category = category;
    entry.timestamp = std::chrono::steady_clock::now();
    entry.processId = 0;
    entry.dismissed = false;
    m_unreadCount++;

    if (m_onLog)
    {
        m_onLog(entry);
    }

    return entry.id;
}

uint64_t LogManager::Trace(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Trace, source, message);
}

uint64_t LogManager::Debug(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Debug, source, message);
}

uint64_t LogManager::Info(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Info, source, message);
}

uint64_t LogManager::Warning(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Warning, source, message);
}

uint64_t LogManager::Error(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Error, source, message);
}

uint64_t LogManager::Fatal(std::wstring_view source, std::wstring_view message) noexcept
{
    return Log(LogLevel::Fatal, source, message);
}

std::vector<const LogEntry*> LogManager::Query(
    LogLevel minLevel,
    std::wstring_view sourceFilter,
    size_t maxResults) const noexcept
{
    std::vector<const LogEntry*> results;
    if (!m_initialized) return results;

    size_t start = (m_count < MaxLogEntries) ? 0 : m_head;
    size_t count = (m_count < MaxLogEntries) ? m_count : MaxLogEntries;

    for (size_t i = 0; i < count && results.size() < maxResults; i++)
    {
        size_t idx = (start + i) % MaxLogEntries;
        const auto& entry = m_entries[idx];

        if (entry.id == 0) continue;
        if (entry.level < minLevel) continue;
        if (!sourceFilter.empty() && entry.source != sourceFilter) continue;

        results.push_back(&entry);
    }

    return results;
}

std::vector<const LogEntry*> LogManager::GetRecent(size_t count) const noexcept
{
    return Query(LogLevel::Trace, L"", count);
}

std::vector<const LogEntry*> LogManager::GetByLevel(LogLevel level, size_t maxResults) const noexcept
{
    return Query(level, L"", maxResults);
}

void LogManager::MarkAllRead() noexcept
{
    m_unreadCount = 0;
}

void LogManager::Dismiss(uint64_t entryId) noexcept
{
    for (size_t i = 0; i < MaxLogEntries; i++)
    {
        if (m_entries[i].id == entryId)
        {
            m_entries[i].dismissed = true;
            break;
        }
    }
}

void LogManager::Clear() noexcept
{
    m_head = 0;
    m_count = 0;
    m_unreadCount = 0;
}

} // namespace DragonOS::Diagnostics
