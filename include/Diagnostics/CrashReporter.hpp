#pragma once

#include <Diagnostics/DiagnosticsTypes.hpp>

#include <Engine/System.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Notifications { class NotificationManager; }

namespace DragonOS::Diagnostics {

class LogManager;

class CrashReporter final : public Engine::System {
public:
    CrashReporter() noexcept = default;
    ~CrashReporter() noexcept override { Shutdown(); }

    CrashReporter(const CrashReporter&) = delete;
    CrashReporter& operator=(const CrashReporter&) = delete;
    CrashReporter(CrashReporter&&) = delete;
    CrashReporter& operator=(CrashReporter&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void SetLogManager(LogManager& logMgr) noexcept { m_pLogMgr = &logMgr; }
    void SetNotificationManager(Notifications::NotificationManager& notifMgr) noexcept
    {
        m_pNotifMgr = &notifMgr;
    }

    uint64_t ReportCrash(
        uint64_t processId,
        std::wstring_view processName,
        std::wstring_view errorMessage,
        std::wstring_view stackTrace = L"",
        std::wstring_view moduleName = L"",
        uint32_t exceptionCode = 0) noexcept;

    const CrashData* Find(uint64_t crashId) const noexcept;
    std::vector<const CrashData*> GetAll() const noexcept;
    size_t GetCrashCount() const noexcept { return m_crashCount; }

    void Dismiss(uint64_t crashId) noexcept;
    void ClearAll() noexcept;

    void SetOnCrashCallback(CrashCallback cb) noexcept { m_onCrash = std::move(cb); }

private:
    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    std::array<CrashData, MaxCrashReports> m_crashes{};
    size_t m_head{ 0 };
    size_t m_crashCount{ 0 };
    LogManager* m_pLogMgr{ nullptr };
    Notifications::NotificationManager* m_pNotifMgr{ nullptr };
    CrashCallback m_onCrash;
    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
