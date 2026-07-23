#include <Diagnostics/DiagnosticDashboardWindow.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>
#include <Input/MouseManager.hpp>
#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::Diagnostics {

DiagnosticDashboardWindow::DiagnosticDashboardWindow() noexcept
{
}

DiagnosticDashboardWindow::~DiagnosticDashboardWindow() noexcept
{
}

void DiagnosticDashboardWindow::SetDependencies(
    WindowManager::DragonWindow& window,
    Theme::ThemeManager& themeManager,
    Input::MouseManager& mouseManager) noexcept
{
    m_pWindow = &window;
    m_pTheme = &themeManager;
    m_pMouse = &mouseManager;
}

void DiagnosticDashboardWindow::UpdateSummary(const DiagnosticSummary& summary) noexcept
{
    m_summary = summary;
}

void DiagnosticDashboardWindow::SetQuickActionCallback(
    std::wstring_view label, std::function<void()> callback) noexcept
{
    QuickAction action;
    action.label = label;
    action.callback = std::move(callback);
    m_quickActions.push_back(std::move(action));
}

void DiagnosticDashboardWindow::Update() noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    auto bounds = m_pWindow->GetBounds();
    m_viewportWidth = bounds.width;
    m_viewportHeight = bounds.height;

    RecalculateLayout();
}

void DiagnosticDashboardWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    RenderHeader(renderer);
    RenderSystemGauges(renderer);
    RenderQuickActions(renderer);
    RenderStatusSummary(renderer);
    RenderStatusBar(renderer);
}

void DiagnosticDashboardWindow::RecalculateLayout() noexcept
{
    float pad = m_layout.padding;
    float totalW = m_viewportWidth;
    float totalH = m_viewportHeight;

    m_layout.headerArea = { pad, pad, totalW - pad * 2.0f, 24.0f };

    float gaugeH = 80.0f;
    float gaugeY = m_layout.headerArea.y + m_layout.headerArea.height + m_layout.sectionSpacing;
    m_layout.gaugesArea = { pad, gaugeY, totalW - pad * 2.0f, gaugeH };

    float actionY = gaugeY + gaugeH + m_layout.sectionSpacing;
    m_layout.actionsArea = { pad, actionY, totalW - pad * 2.0f, 40.0f };

    float summaryY = actionY + m_layout.actionsArea.height + m_layout.sectionSpacing;
    float summaryH = totalH - summaryY - pad - 20.0f;
    m_layout.summaryArea = { pad, summaryY, totalW - pad * 2.0f, summaryH };

    m_layout.statusBarArea = { pad, totalH - 18.0f, totalW - pad * 2.0f, 16.0f };
}

void DiagnosticDashboardWindow::RenderHeader(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F rect = { m_layout.headerArea.x, m_layout.headerArea.y,
                          m_layout.headerArea.x + m_layout.headerArea.width,
                          m_layout.headerArea.y + m_layout.headerArea.height };

    renderer.DrawText(L"Diagnostic Dashboard", rect, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void DiagnosticDashboardWindow::RenderSystemGauges(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);

    D2D1_RECT_F area = { m_layout.gaugesArea.x, m_layout.gaugesArea.y,
                          m_layout.gaugesArea.x + m_layout.gaugesArea.width,
                          m_layout.gaugesArea.y + m_layout.gaugesArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.1f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    float gaugeW = (area.right - area.left - 12.0f) / 4.0f;
    float gaugeY = area.top + 4.0f;
    float gaugeH = area.bottom - area.top - 8.0f;

    D2D1_RECT_F cpuGauge = { area.left + 2.0f, gaugeY, area.left + 2.0f + gaugeW, gaugeY + gaugeH };
    RenderMiniGauge(renderer, cpuGauge, m_summary.cpuUsagePercent / 100.0,
                    L"CPU", Graphics::Color{ 0.1f, 0.6f, 0.9f });

    D2D1_RECT_F memGauge = { cpuGauge.right + 2.0f, gaugeY, cpuGauge.right + 2.0f + gaugeW, gaugeY + gaugeH };
    RenderMiniGauge(renderer, memGauge, m_summary.memoryUsagePercent / 100.0,
                    L"Memory", Graphics::Color{ 0.2f, 0.8f, 0.3f });

    D2D1_RECT_F procGauge = { memGauge.right + 2.0f, gaugeY, memGauge.right + 2.0f + gaugeW, gaugeY + gaugeH };
    double procPercent = (std::min)(static_cast<double>(m_summary.processCount) / 100.0, 1.0);
    RenderMiniGauge(renderer, procGauge, procPercent,
                    L"Processes", Graphics::Color{ 0.9f, 0.6f, 0.1f });

    D2D1_RECT_F notifGauge = { procGauge.right + 2.0f, gaugeY, procGauge.right + 2.0f + gaugeW, gaugeY + gaugeH };
    double notifPercent = (std::min)(static_cast<double>(m_summary.unreadNotificationCount) / 50.0, 1.0);
    RenderMiniGauge(renderer, notifGauge, notifPercent,
                    L"Notifications", Graphics::Color{ 0.8f, 0.3f, 0.5f });
}

void DiagnosticDashboardWindow::RenderQuickActions(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);

    D2D1_RECT_F area = { m_layout.actionsArea.x, m_layout.actionsArea.y,
                          m_layout.actionsArea.x + m_layout.actionsArea.width,
                          m_layout.actionsArea.y + m_layout.actionsArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.1f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    float btnW = (area.right - area.left - 8.0f) / 4.0f;
    float btnY = area.top + 4.0f;
    float btnH = area.bottom - area.top - 8.0f;

    m_hoveredActionIdx = -1;
    if (m_pMouse)
    {
        auto mousePos = m_pMouse->GetPosition();
        float mx = mousePos.x - m_pWindow->GetX();
        float my = mousePos.y - m_pWindow->GetY();

        for (size_t i = 0; i < m_quickActions.size(); i++)
        {
            D2D1_RECT_F btnBounds = { area.left + 2.0f + static_cast<float>(i) * (btnW + 2.0f), btnY,
                                       area.left + 2.0f + static_cast<float>(i) * (btnW + 2.0f) + btnW, btnY + btnH };
            if (mx >= btnBounds.left && mx < btnBounds.right &&
                my >= btnBounds.top && my < btnBounds.bottom)
            {
                m_hoveredActionIdx = static_cast<int>(i);
                break;
            }
        }
    }

    for (size_t i = 0; i < m_quickActions.size(); i++)
    {
        D2D1_RECT_F btnBounds = { area.left + 2.0f + static_cast<float>(i) * (btnW + 2.0f), btnY,
                                   area.left + 2.0f + static_cast<float>(i) * (btnW + 2.0f) + btnW, btnY + btnH };
        RenderQuickActionButton(renderer, btnBounds, m_quickActions[i].label,
                                static_cast<int>(i) == m_hoveredActionIdx);
    }
}

void DiagnosticDashboardWindow::RenderStatusSummary(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.summaryArea.x, m_layout.summaryArea.y,
                          m_layout.summaryArea.x + m_layout.summaryArea.width,
                          m_layout.summaryArea.y + m_layout.summaryArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.1f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    float y = area.top + 4.0f;
    float lineH = 16.0f;

    auto renderLine = [&](const std::wstring& text)
    {
        D2D1_RECT_F lineBounds = { area.left + 4.0f, y, area.right - 4.0f, y + lineH };
        renderer.DrawText(text, lineBounds, { textColor.r, textColor.g, textColor.b, textColor.a });
        y += lineH;
    };

    renderLine(L"System Status Summary");
    y += 2.0f;
    renderLine(L"  CPU:       " + std::to_wstring(static_cast<int>(m_summary.cpuUsagePercent)) + L"%");
    renderLine(L"  Memory:    " + std::to_wstring(static_cast<int>(m_summary.memoryUsagePercent)) + L"% (" +
               std::to_wstring(static_cast<int>(m_summary.memoryUsedMB)) + L" / " +
               std::to_wstring(static_cast<int>(m_summary.memoryTotalMB)) + L" MB)");
    renderLine(L"  Processes: " + std::to_wstring(m_summary.processCount));
    renderLine(L"  Apps:      " + std::to_wstring(m_summary.runningAppCount));
    renderLine(L"  Notifications: " + std::to_wstring(m_summary.activeNotificationCount) +
               L" active, " + std::to_wstring(m_summary.unreadNotificationCount) + L" unread");
    renderLine(L"  Security:  " + std::wstring(m_summary.securityEnabled ? L"Enabled" : L"Disabled"));
}

void DiagnosticDashboardWindow::RenderStatusBar(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.statusBarArea.x, m_layout.statusBarArea.y,
                          m_layout.statusBarArea.x + m_layout.statusBarArea.width,
                          m_layout.statusBarArea.y + m_layout.statusBarArea.height };

    std::wstring status = L"System: " +
        std::wstring(m_summary.securityEnabled ? L"Secure" : L"Unsecured") +
        L"  |  Logs: " + std::to_wstring(m_summary.unreadLogCount) + L" unread";

    renderer.DrawText(status, area, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void DiagnosticDashboardWindow::RenderMiniGauge(
    Graphics::Renderer& renderer,
    const D2D1_RECT_F& bounds,
    double percent,
    const std::wstring& label,
    const Graphics::Color& color) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    renderer.FillRectangle(bounds, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.2f);
    renderer.DrawRectangle(bounds, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    float barH = 8.0f;
    D2D1_RECT_F barBounds = { bounds.left + 4.0f, bounds.bottom - barH - 4.0f,
                               bounds.right - 4.0f, bounds.bottom - 4.0f };
    renderer.FillRectangle(barBounds, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.3f);

    if (percent > 0.0)
    {
        D2D1_RECT_F fillBounds = { barBounds.left, barBounds.top,
                                    barBounds.left + (barBounds.right - barBounds.left) * static_cast<float>(percent),
                                    barBounds.bottom };
        renderer.FillRectangle(fillBounds, color, 0.7f);
    }

    D2D1_RECT_F labelBounds = { bounds.left + 4.0f, bounds.top + 4.0f, bounds.right - 4.0f, bounds.top + 20.0f };
    renderer.DrawText(label, labelBounds, { textColor.r, textColor.g, textColor.b, textColor.a });

    std::wstring pctText = std::to_wstring(static_cast<int>(percent * 100.0)) + L"%";
    D2D1_RECT_F pctBounds = { bounds.left + 4.0f, bounds.top + 20.0f, bounds.right - 4.0f, bounds.top + 36.0f };
    renderer.DrawText(pctText, pctBounds, color);
}

void DiagnosticDashboardWindow::RenderQuickActionButton(
    Graphics::Renderer& renderer,
    const D2D1_RECT_F& bounds,
    const std::wstring& label,
    bool hovered) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto accentColor = palette.Get(Theme::SemanticColor::Accent);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    if (hovered)
    {
        renderer.FillRectangle(bounds, { accentColor.r, accentColor.g, accentColor.b, accentColor.a }, 0.3f);
    }

    renderer.DrawRectangle(bounds, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);
    renderer.DrawText(label, bounds, { textColor.r, textColor.g, textColor.b, textColor.a });
}

} // namespace DragonOS::Diagnostics
