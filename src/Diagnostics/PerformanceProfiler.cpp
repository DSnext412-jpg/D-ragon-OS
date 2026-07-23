#include <Diagnostics/PerformanceProfiler.hpp>

namespace DragonOS::Diagnostics {

bool PerformanceProfiler::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    m_frameStart = std::chrono::steady_clock::now();
    m_avgFrameTime = 0.0;
    m_maxFrameTime = 0.0;
    m_fps = 0.0;
    m_frameCount = 0;
    m_totalSamples = 0;
    m_fpsTimer = 0.0f;
    m_initialized = true;
    return true;
}

void PerformanceProfiler::Shutdown() noexcept
{
    if (!m_initialized) return;
    m_sessions.clear();
    m_metricHistory.clear();
    m_initialized = false;
}

void PerformanceProfiler::Update(float deltaTime) noexcept
{
    if (!m_initialized) return;

    m_fpsTimer += deltaTime;

    if (m_fpsTimer >= 1.0f)
    {
        m_fps = static_cast<double>(m_frameCount) / m_fpsTimer;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }
}

void PerformanceProfiler::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void PerformanceProfiler::Resize(float /*width*/, float /*height*/) noexcept
{
}

uint64_t PerformanceProfiler::StartSession(std::wstring_view name) noexcept
{
    if (!m_initialized) return 0;

    ProfilingSession session;
    session.sessionId = NextSessionId();
    session.name = name;
    session.startTime = std::chrono::steady_clock::now();
    session.active = true;

    m_sessions.push_back(std::move(session));
    return m_sessions.back().sessionId;
}

bool PerformanceProfiler::EndSession(uint64_t sessionId) noexcept
{
    for (auto& session : m_sessions)
    {
        if (session.sessionId == sessionId && session.active)
        {
            session.endTime = std::chrono::steady_clock::now();
            session.active = false;
            return true;
        }
    }
    return false;
}

const ProfilingSession* PerformanceProfiler::GetSession(uint64_t sessionId) const noexcept
{
    for (const auto& session : m_sessions)
    {
        if (session.sessionId == sessionId)
            return &session;
    }
    return nullptr;
}

std::vector<const ProfilingSession*> PerformanceProfiler::GetAllSessions() const noexcept
{
    std::vector<const ProfilingSession*> result;
    result.reserve(m_sessions.size());
    for (const auto& session : m_sessions)
    {
        result.push_back(&session);
    }
    return result;
}

void PerformanceProfiler::RecordMetric(MetricType type, double value, std::wstring_view name) noexcept
{
    if (!m_initialized) return;

    if (m_metricHistory.size() >= MaxMetricSamples)
    {
        m_metricHistory.erase(m_metricHistory.begin());
    }

    MetricSample sample;
    sample.id = m_totalSamples++;
    sample.type = type;
    sample.value = value;
    sample.name = name;
    sample.timestamp = std::chrono::steady_clock::now();

    m_metricHistory.push_back(sample);

    for (auto& session : m_sessions)
    {
        if (session.active)
        {
            session.metrics.push_back(sample);
        }
    }
}

void PerformanceProfiler::EndFrame() noexcept
{
    if (!m_initialized) return;

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> frameTime = now - m_frameStart;
    double ft = frameTime.count() * 1000.0;

    if (m_frameCount == 0)
    {
        m_avgFrameTime = ft;
    }
    else
    {
        m_avgFrameTime = m_avgFrameTime * 0.95 + ft * 0.05;
    }

    if (ft > m_maxFrameTime)
    {
        m_maxFrameTime = ft;
    }

    m_frameCount++;
    m_frameStart = now;
}

} // namespace DragonOS::Diagnostics
