#include <Diagnostics/CrashReporter.hpp>
#include <Diagnostics/LogManager.hpp>
#include <Notifications/NotificationManager.hpp>

namespace DragonOS::Diagnostics {

bool CrashReporter::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    m_head = 0;
    m_crashCount = 0;
    m_initialized = true;
    return true;
}

void CrashReporter::Shutdown() noexcept
{
    if (!m_initialized) return;
    m_onCrash = nullptr;
    m_pLogMgr = nullptr;
    m_pNotifMgr = nullptr;
    m_initialized = false;
}

void CrashReporter::Update(float /*deltaTime*/) noexcept
{
}

void CrashReporter::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void CrashReporter::Resize(float /*width*/, float /*height*/) noexcept
{
}

uint64_t CrashReporter::ReportCrash(
    uint64_t processId,
    std::wstring_view processName,
    std::wstring_view errorMessage,
    std::wstring_view stackTrace,
    std::wstring_view moduleName,
    uint32_t exceptionCode) noexcept
{
    if (!m_initialized) return 0;

    size_t idx = m_head;
    m_head = (m_head + 1) % MaxCrashReports;
    if (m_crashCount < MaxCrashReports)
    {
        m_crashCount++;
    }

    auto& crash = m_crashes[idx];
    crash.crashId = NextId();
    crash.processId = processId;
    crash.processName = processName;
    crash.errorMessage = errorMessage;
    crash.stackTrace = stackTrace;
    crash.moduleName = moduleName;
    crash.exceptionCode = exceptionCode;
    crash.timestamp = std::chrono::steady_clock::now();
    crash.reported = false;

    if (m_pLogMgr)
    {
        m_pLogMgr->Error(processName, L"Crash: " + std::wstring(errorMessage));
    }

    if (m_pNotifMgr)
    {
        Notifications::Notification notif;
        notif.title = L"Application Crash";
        notif.message = std::wstring(processName) + L" has stopped working.";
        notif.source = L"CrashReporter";
        notif.severity = Notifications::NotificationSeverity::Error;
        notif.dismissMode = Notifications::NotificationDismiss::Manual;
        m_pNotifMgr->Show(std::move(notif));
    }

    if (m_onCrash)
    {
        m_onCrash(crash);
    }

    return crash.crashId;
}

const CrashData* CrashReporter::Find(uint64_t crashId) const noexcept
{
    for (size_t i = 0; i < MaxCrashReports; i++)
    {
        if (m_crashes[i].crashId == crashId)
            return &m_crashes[i];
    }
    return nullptr;
}

std::vector<const CrashData*> CrashReporter::GetAll() const noexcept
{
    std::vector<const CrashData*> result;
    result.reserve(m_crashCount);
    for (size_t i = 0; i < MaxCrashReports; i++)
    {
        if (m_crashes[i].crashId != 0)
        {
            result.push_back(&m_crashes[i]);
        }
    }
    return result;
}

void CrashReporter::Dismiss(uint64_t crashId) noexcept
{
    for (size_t i = 0; i < MaxCrashReports; i++)
    {
        if (m_crashes[i].crashId == crashId)
        {
            m_crashes[i].reported = true;
            break;
        }
    }
}

void CrashReporter::ClearAll() noexcept
{
    m_head = 0;
    m_crashCount = 0;
    for (auto& crash : m_crashes)
    {
        crash.crashId = 0;
    }
}

} // namespace DragonOS::Diagnostics
