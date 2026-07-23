#pragma once

#include <Diagnostics/DiagnosticsTypes.hpp>

#include <Engine/System.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Diagnostics {

class PerformanceProfiler final : public Engine::System {
public:
    PerformanceProfiler() noexcept = default;
    ~PerformanceProfiler() noexcept override { Shutdown(); }

    PerformanceProfiler(const PerformanceProfiler&) = delete;
    PerformanceProfiler& operator=(const PerformanceProfiler&) = delete;
    PerformanceProfiler(PerformanceProfiler&&) = delete;
    PerformanceProfiler& operator=(PerformanceProfiler&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    uint64_t StartSession(std::wstring_view name) noexcept;
    bool EndSession(uint64_t sessionId) noexcept;
    const ProfilingSession* GetSession(uint64_t sessionId) const noexcept;
    std::vector<const ProfilingSession*> GetAllSessions() const noexcept;

    void RecordMetric(MetricType type, double value, std::wstring_view name = L"") noexcept;

    void BeginFrame() noexcept { m_frameStart = std::chrono::steady_clock::now(); }
    void EndFrame() noexcept;

    double GetAverageFrameTime() const noexcept { return m_avgFrameTime; }
    double GetMaxFrameTime() const noexcept { return m_maxFrameTime; }
    double GetFps() const noexcept { return m_fps; }

    double GetCpuUsage() const noexcept { return m_cpuUsage; }
    void SetCpuUsage(double usage) noexcept { m_cpuUsage = usage; }

    size_t GetTotalSamples() const noexcept { return m_totalSamples; }

private:
    uint64_t NextSessionId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    std::vector<ProfilingSession> m_sessions;
    std::vector<MetricSample> m_metricHistory;

    std::chrono::steady_clock::time_point m_frameStart;
    double m_avgFrameTime{ 0.0 };
    double m_maxFrameTime{ 0.0 };
    double m_fps{ 0.0 };
    double m_cpuUsage{ 0.0 };

    size_t m_totalSamples{ 0 };
    size_t m_frameCount{ 0 };
    float m_fpsTimer{ 0.0f };

    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
