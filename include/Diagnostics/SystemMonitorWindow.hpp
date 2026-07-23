#pragma once

#include <Diagnostics/DiagnosticsTypes.hpp>

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>

#include <d2d1.h>

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class MouseManager; }
namespace DragonOS::WindowManager { class DragonWindow; }
namespace DragonOS::Process { class ProcessManager; }

namespace DragonOS::Diagnostics {

class SystemMonitorWindow final {
public:
    SystemMonitorWindow() noexcept;
    ~SystemMonitorWindow() noexcept;

    SystemMonitorWindow(const SystemMonitorWindow&) = delete;
    SystemMonitorWindow& operator=(const SystemMonitorWindow&) = delete;
    SystemMonitorWindow(SystemMonitorWindow&&) = delete;
    SystemMonitorWindow& operator=(SystemMonitorWindow&&) = delete;

    void SetDependencies(
        WindowManager::DragonWindow& window,
        Theme::ThemeManager& themeManager,
        Input::MouseManager& mouseManager,
        Process::ProcessManager& procManager) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept
    {
        m_pWindow = &window;
    }

    void PushMetric(MetricType type, double value, std::wstring_view /*name*/ = L"") noexcept;

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;

    [[nodiscard]] uint64_t GetWindowId() const noexcept;

    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"System Monitor";
        return name;
    }

    void UpdateMetrics(
        double cpu,
        double memoryUsedMB,
        double memoryTotalMB,
        size_t processCount) noexcept;

private:
    void RecalculateLayout() noexcept;

    void RenderHeader(Graphics::Renderer& renderer) noexcept;
    void RenderCpuSection(Graphics::Renderer& renderer) noexcept;
    void RenderMemorySection(Graphics::Renderer& renderer) noexcept;
    void RenderProcessList(Graphics::Renderer& renderer) noexcept;
    void RenderStatusBar(Graphics::Renderer& renderer) noexcept;

    void RenderMiniGraph(Graphics::Renderer& renderer, const D2D1_RECT_F& bounds,
                         const std::vector<double>& data, const Graphics::Color& color) noexcept;
    void RenderGauge(Graphics::Renderer& renderer, const D2D1_RECT_F& bounds,
                     double percent, const Graphics::Color& color) noexcept;

    struct MetricHistory {
        std::vector<double> samples;
        size_t maxSamples{ 120 };
    };

    MetricHistory m_cpuHistory;
    MetricHistory m_memHistory;
    double m_currentCpu{ 0.0 };
    double m_currentMemUsedMB{ 0.0 };
    double m_currentMemTotalMB{ 0.0 };
    double m_currentMemPercent{ 0.0 };
    size_t m_currentProcessCount{ 0 };
    float m_updateTimer{ 0.0f };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };

    struct Layout {
        Input::Bounds headerArea{};
        Input::Bounds cpuArea{};
        Input::Bounds memoryArea{};
        Input::Bounds processListArea{};
        Input::Bounds statusBarArea{};
        float padding{ 8.0f };
        float sectionSpacing{ 4.0f };
    };
    Layout m_layout{};

    WindowManager::DragonWindow* m_pWindow{ nullptr };
    Theme::ThemeManager* m_pTheme{ nullptr };
    Input::MouseManager* m_pMouse{ nullptr };
    Process::ProcessManager* m_pProcMgr{ nullptr };

    int m_hoveredProcessIdx{ -1 };
    float m_scrollOffset{ 0.0f };

    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
