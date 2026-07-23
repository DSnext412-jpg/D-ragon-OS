#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace DragonOS::Diagnostics {

enum class LogLevel : uint8_t {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

enum class MetricType : uint8_t {
    CpuUsage,
    MemoryUsage,
    DiskUsage,
    NetworkUsage,
    GpuUsage,
    FrameTime,
    ProcessCount,
    ThreadCount,
    Custom
};

enum class DiagnosticCategory : uint8_t {
    System,
    Performance,
    Security,
    Applications,
    Services,
    Network,
    Storage,
    Custom
};

struct LogEntry final {
    uint64_t                    id{ 0 };
    LogLevel                    level{ LogLevel::Info };
    std::wstring                source;
    std::wstring                message;
    std::wstring                category;
    std::chrono::steady_clock::time_point timestamp;
    uint64_t                    processId{ 0 };
    bool                        dismissed{ false };
};

struct MetricSample final {
    uint64_t                    id{ 0 };
    MetricType                  type{ MetricType::Custom };
    double                      value{ 0.0 };
    std::wstring                name;
    std::chrono::steady_clock::time_point timestamp;
};

struct CrashData final {
    uint64_t                    crashId{ 0 };
    uint64_t                    processId{ 0 };
    std::wstring                processName;
    std::wstring                errorMessage;
    std::wstring                stackTrace;
    std::wstring                moduleName;
    uint32_t                    exceptionCode{ 0 };
    std::chrono::steady_clock::time_point timestamp;
    bool                        reported{ false };
};

struct DiagnosticSummary final {
    double                      cpuUsagePercent{ 0.0 };
    double                      memoryUsagePercent{ 0.0 };
    double                      memoryUsedMB{ 0.0 };
    double                      memoryTotalMB{ 0.0 };
    size_t                      processCount{ 0 };
    size_t                      runningAppCount{ 0 };
    size_t                      activeNotificationCount{ 0 };
    size_t                      unreadNotificationCount{ 0 };
    size_t                      unreadLogCount{ 0 };
    bool                        securityEnabled{ false };
};

struct ProfilingSession final {
    uint64_t                    sessionId{ 0 };
    std::wstring                name;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    bool                        active{ false };
    std::vector<MetricSample>   metrics;
};

using LogCallback = std::function<void(const LogEntry&)>;
using CrashCallback = std::function<void(const CrashData&)>;

static constexpr size_t MaxLogEntries = 2000;
static constexpr size_t MaxMetricSamples = 10000;
static constexpr size_t MaxCrashReports = 50;

} // namespace DragonOS::Diagnostics
