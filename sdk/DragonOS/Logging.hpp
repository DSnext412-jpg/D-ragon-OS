#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>
#include <string_view>

namespace dragonos::sdk {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

class Logger {
public:
    static Logger& Get() noexcept
    {
        static Logger instance;
        return instance;
    }

    void SetOutput(FILE* file) noexcept { m_output = file; }

    void Log(LogLevel level, std::wstring_view message) noexcept
    {
        if (!m_output) { m_output = stderr; }

        const wchar_t* prefix = L"[?] ";
        switch (level)
        {
        case LogLevel::Debug:   prefix = L"[DBG] "; break;
        case LogLevel::Info:    prefix = L"[INF] "; break;
        case LogLevel::Warning: prefix = L"[WRN] "; break;
        case LogLevel::Error:   prefix = L"[ERR] "; break;
        }

        fwprintf(m_output, L"%s%.*s\n", prefix,
                 static_cast<int>(message.size()), message.data());
        fflush(m_output);
    }

    void Debug(std::wstring_view msg) noexcept { Log(LogLevel::Debug, msg); }
    void Info(std::wstring_view msg) noexcept { Log(LogLevel::Info, msg); }
    void Warning(std::wstring_view msg) noexcept { Log(LogLevel::Warning, msg); }
    void Error(std::wstring_view msg) noexcept { Log(LogLevel::Error, msg); }

private:
    Logger() noexcept = default;
    FILE* m_output{ nullptr };
};

} // namespace dragonos::sdk
