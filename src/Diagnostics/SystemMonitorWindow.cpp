#include <Diagnostics/SystemMonitorWindow.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>
#include <Input/MouseManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <Process/ProcessManager.hpp>

namespace DragonOS::Diagnostics {

SystemMonitorWindow::SystemMonitorWindow() noexcept
{
    m_cpuHistory.maxSamples = 120;
    m_memHistory.maxSamples = 120;
    m_cpuHistory.samples.reserve(120);
    m_memHistory.samples.reserve(120);
}

SystemMonitorWindow::~SystemMonitorWindow() noexcept
{
}

void SystemMonitorWindow::SetDependencies(
    WindowManager::DragonWindow& window,
    Theme::ThemeManager& themeManager,
    Input::MouseManager& mouseManager,
    Process::ProcessManager& procManager) noexcept
{
    m_pWindow = &window;
    m_pTheme = &themeManager;
    m_pMouse = &mouseManager;
    m_pProcMgr = &procManager;
}

void SystemMonitorWindow::PushMetric(MetricType type, double value, std::wstring_view /*name*/) noexcept
{
    switch (type)
    {
    case MetricType::CpuUsage:
        m_currentCpu = value;
        m_cpuHistory.samples.push_back(value);
        if (m_cpuHistory.samples.size() > m_cpuHistory.maxSamples)
            m_cpuHistory.samples.erase(m_cpuHistory.samples.begin());
        break;
    case MetricType::MemoryUsage:
        m_currentMemPercent = value;
        m_memHistory.samples.push_back(value);
        if (m_memHistory.samples.size() > m_memHistory.maxSamples)
            m_memHistory.samples.erase(m_memHistory.samples.begin());
        break;
    default:
        break;
    }
}

void SystemMonitorWindow::UpdateMetrics(
    double cpu, double memoryUsedMB, double memoryTotalMB, size_t processCount) noexcept
{
    m_currentCpu = cpu;
    m_currentMemUsedMB = memoryUsedMB;
    m_currentMemTotalMB = memoryTotalMB;
    m_currentMemPercent = (memoryTotalMB > 0.0) ? (memoryUsedMB / memoryTotalMB) * 100.0 : 0.0;
    m_currentProcessCount = processCount;

    m_cpuHistory.samples.push_back(m_currentCpu);
    if (m_cpuHistory.samples.size() > m_cpuHistory.maxSamples)
        m_cpuHistory.samples.erase(m_cpuHistory.samples.begin());

    m_memHistory.samples.push_back(m_currentMemPercent);
    if (m_memHistory.samples.size() > m_memHistory.maxSamples)
        m_memHistory.samples.erase(m_memHistory.samples.begin());
}

uint64_t SystemMonitorWindow::GetWindowId() const noexcept
{
    return m_pWindow ? m_pWindow->GetId() : 0;
}

void SystemMonitorWindow::Update() noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    auto bounds = m_pWindow->GetBounds();
    m_viewportWidth = bounds.width;
    m_viewportHeight = bounds.height;

    RecalculateLayout();
}

void SystemMonitorWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    RenderHeader(renderer);
    RenderCpuSection(renderer);
    RenderMemorySection(renderer);
    RenderProcessList(renderer);
    RenderStatusBar(renderer);
}

void SystemMonitorWindow::RecalculateLayout() noexcept
{
    float pad = m_layout.padding;
    float totalW = m_viewportWidth;
    float totalH = m_viewportHeight;

    m_layout.headerArea = { pad, pad, totalW - pad * 2.0f, 24.0f };

    float gaugeW = (totalW - pad * 3.0f) / 2.0f;
    float gaugeH = 120.0f;
    float gaugeY = m_layout.headerArea.y + m_layout.headerArea.height + m_layout.sectionSpacing;

    m_layout.cpuArea = { pad, gaugeY, gaugeW, gaugeH };
    m_layout.memoryArea = { pad + gaugeW + pad, gaugeY, gaugeW, gaugeH };

    float listY = gaugeY + gaugeH + m_layout.sectionSpacing;
    float listH = totalH - listY - pad - 20.0f;
    m_layout.processListArea = { pad, listY, totalW - pad * 2.0f, listH };

    m_layout.statusBarArea = { pad, totalH - 18.0f, totalW - pad * 2.0f, 16.0f };
}

void SystemMonitorWindow::RenderHeader(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F rect = { m_layout.headerArea.x, m_layout.headerArea.y,
                          m_layout.headerArea.x + m_layout.headerArea.width,
                          m_layout.headerArea.y + m_layout.headerArea.height };

    renderer.DrawText(L"System Monitor", rect, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void SystemMonitorWindow::RenderCpuSection(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.cpuArea.x, m_layout.cpuArea.y,
                          m_layout.cpuArea.x + m_layout.cpuArea.width,
                          m_layout.cpuArea.y + m_layout.cpuArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.15f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    D2D1_RECT_F labelBounds = { area.left + 4.0f, area.top + 2.0f, area.right - 4.0f, area.top + 18.0f };
    std::wstring cpuLabel = L"CPU: " + std::to_wstring(static_cast<int>(m_currentCpu)) + L"%";
    renderer.DrawText(cpuLabel, labelBounds, { textColor.r, textColor.g, textColor.b, textColor.a });

    D2D1_RECT_F gaugeBounds = { area.left + 4.0f, area.top + 20.0f, area.right - 4.0f, area.top + 50.0f };
    RenderGauge(renderer, gaugeBounds, m_currentCpu / 100.0, Graphics::Color{ 0.1f, 0.6f, 0.9f });

    D2D1_RECT_F graphBounds = { area.left + 4.0f, area.top + 54.0f, area.right - 4.0f, area.bottom - 4.0f };
    RenderMiniGraph(renderer, graphBounds, m_cpuHistory.samples, Graphics::Color{ 0.1f, 0.6f, 0.9f });
}

void SystemMonitorWindow::RenderMemorySection(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.memoryArea.x, m_layout.memoryArea.y,
                          m_layout.memoryArea.x + m_layout.memoryArea.width,
                          m_layout.memoryArea.y + m_layout.memoryArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.15f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    D2D1_RECT_F labelBounds = { area.left + 4.0f, area.top + 2.0f, area.right - 4.0f, area.top + 18.0f };
    std::wstring memLabel = L"Memory: " +
        std::to_wstring(static_cast<int>(m_currentMemUsedMB)) + L" MB / " +
        std::to_wstring(static_cast<int>(m_currentMemTotalMB)) + L" MB";
    renderer.DrawText(memLabel, labelBounds, { textColor.r, textColor.g, textColor.b, textColor.a });

    D2D1_RECT_F gaugeBounds = { area.left + 4.0f, area.top + 20.0f, area.right - 4.0f, area.top + 50.0f };
    RenderGauge(renderer, gaugeBounds, m_currentMemPercent / 100.0, Graphics::Color{ 0.2f, 0.8f, 0.3f });

    D2D1_RECT_F graphBounds = { area.left + 4.0f, area.top + 54.0f, area.right - 4.0f, area.bottom - 4.0f };
    RenderMiniGraph(renderer, graphBounds, m_memHistory.samples, Graphics::Color{ 0.2f, 0.8f, 0.3f });
}

void SystemMonitorWindow::RenderProcessList(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);
    auto accentColor = palette.Get(Theme::SemanticColor::Accent);

    D2D1_RECT_F area = { m_layout.processListArea.x, m_layout.processListArea.y,
                          m_layout.processListArea.x + m_layout.processListArea.width,
                          m_layout.processListArea.y + m_layout.processListArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.1f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    D2D1_RECT_F headerBounds = { area.left + 4.0f, area.top + 2.0f, area.right - 4.0f, area.top + 18.0f };
    std::wstring procHeader = L"Processes (" + std::to_wstring(m_currentProcessCount) + L")";
    renderer.DrawText(procHeader, headerBounds, { textColor.r, textColor.g, textColor.b, textColor.a });

    if (!m_pProcMgr) return;

    auto processes = m_pProcMgr->GetAll();
    float itemY = area.top + 20.0f;
    float itemH = 16.0f;
    float itemW = area.right - area.left - 8.0f;

    m_hoveredProcessIdx = -1;
    if (m_pMouse)
    {
        auto mousePos = m_pMouse->GetPosition();
        float mx = mousePos.x - m_pWindow->GetX();
        float my = mousePos.y - m_pWindow->GetY();

        for (size_t i = 0; i < processes.size(); i++)
        {
            D2D1_RECT_F itemBounds = { area.left + 2.0f, itemY + i * itemH,
                                        area.left + 2.0f + itemW, itemY + i * itemH + itemH };
            if (mx >= itemBounds.left && mx < itemBounds.right &&
                my >= itemBounds.top && my < itemBounds.bottom)
            {
                m_hoveredProcessIdx = static_cast<int>(i);
                break;
            }
        }
    }

    for (size_t i = 0; i < processes.size(); i++)
    {
        float y = itemY + static_cast<float>(i) * itemH;
        if (y + itemH > area.bottom) break;
        if (y < area.top) continue;

        D2D1_RECT_F itemBounds = { area.left + 2.0f, y, area.left + 2.0f + itemW, y + itemH };

        if (static_cast<int>(i) == m_hoveredProcessIdx)
        {
            renderer.FillRectangle(itemBounds, { accentColor.r, accentColor.g, accentColor.b, accentColor.a }, 0.2f);
        }

        auto* proc = processes[i];
        std::wstring procText = proc->GetName();
        renderer.DrawText(procText, itemBounds, { textColor.r, textColor.g, textColor.b, textColor.a });
    }
}

void SystemMonitorWindow::RenderStatusBar(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.statusBarArea.x, m_layout.statusBarArea.y,
                          m_layout.statusBarArea.x + m_layout.statusBarArea.width,
                          m_layout.statusBarArea.y + m_layout.statusBarArea.height };

    std::wstring status = L"CPU: " + std::to_wstring(static_cast<int>(m_currentCpu)) +
        L"%  |  Memory: " + std::to_wstring(static_cast<int>(m_currentMemPercent)) + L"%  |  Processes: " +
        std::to_wstring(m_currentProcessCount);

    renderer.DrawText(status, area, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void SystemMonitorWindow::RenderMiniGraph(
    Graphics::Renderer& renderer,
    const D2D1_RECT_F& bounds,
    const std::vector<double>& data,
    const Graphics::Color& color) noexcept
{
    if (data.size() < 2) return;

    double minVal = 0.0;
    double maxVal = 100.0;

    float stepX = (bounds.right - bounds.left) / static_cast<float>(data.size() - 1);

    for (size_t i = 1; i < data.size(); i++)
    {
        float x1 = bounds.left + static_cast<float>(i - 1) * stepX;
        float y1 = bounds.bottom - static_cast<float>((data[i - 1] - minVal) / (maxVal - minVal)) * (bounds.bottom - bounds.top);
        float x2 = bounds.left + static_cast<float>(i) * stepX;
        float y2 = bounds.bottom - static_cast<float>((data[i] - minVal) / (maxVal - minVal)) * (bounds.bottom - bounds.top);

        y1 = (std::max)(bounds.top, (std::min)(bounds.bottom, y1));
        y2 = (std::max)(bounds.top, (std::min)(bounds.bottom, y2));

        renderer.DrawLine(
            D2D1_POINT_2F{ x1, y1 },
            D2D1_POINT_2F{ x2, y2 },
            color,
            1.0f);
    }
}

void SystemMonitorWindow::RenderGauge(
    Graphics::Renderer& renderer,
    const D2D1_RECT_F& bounds,
    double percent,
    const Graphics::Color& color) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);

    renderer.FillRectangle(bounds, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.3f);
    renderer.DrawRectangle(bounds, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    if (percent > 0.0)
    {
        D2D1_RECT_F fillBounds = {
            bounds.left + 1.0f,
            bounds.top + 1.0f,
            bounds.left + 1.0f + (bounds.right - bounds.left - 2.0f) * static_cast<float>(percent),
            bounds.bottom - 1.0f
        };
        renderer.FillRectangle(fillBounds, color, 0.7f);
    }
}

} // namespace DragonOS::Diagnostics
