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
namespace DragonOS::Notifications { class NotificationManager; }

namespace DragonOS::Diagnostics {

class DiagnosticDashboardWindow final {
public:
    DiagnosticDashboardWindow() noexcept;
    ~DiagnosticDashboardWindow() noexcept;

    DiagnosticDashboardWindow(const DiagnosticDashboardWindow&) = delete;
    DiagnosticDashboardWindow& operator=(const DiagnosticDashboardWindow&) = delete;
    DiagnosticDashboardWindow(DiagnosticDashboardWindow&&) = delete;
    DiagnosticDashboardWindow& operator=(DiagnosticDashboardWindow&&) = delete;

    void SetDependencies(
        WindowManager::DragonWindow& window,
        Theme::ThemeManager& themeManager,
        Input::MouseManager& mouseManager) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept
    {
        m_pWindow = &window;
    }

    void UpdateSummary(const DiagnosticSummary& summary) noexcept;
    void SetQuickActionCallback(std::wstring_view label, std::function<void()> callback) noexcept;

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;

    [[nodiscard]] uint64_t GetWindowId() const noexcept;

    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"Diagnostic Dashboard";
        return name;
    }

private:
    void RecalculateLayout() noexcept;

    void RenderHeader(Graphics::Renderer& renderer) noexcept;
    void RenderSystemGauges(Graphics::Renderer& renderer) noexcept;
    void RenderQuickActions(Graphics::Renderer& renderer) noexcept;
    void RenderStatusSummary(Graphics::Renderer& renderer) noexcept;
    void RenderStatusBar(Graphics::Renderer& renderer) noexcept;

    void RenderMiniGauge(Graphics::Renderer& renderer, const D2D1_RECT_F& bounds,
                         double percent, const std::wstring& label,
                         const Graphics::Color& color) noexcept;
    void RenderQuickActionButton(Graphics::Renderer& renderer, const D2D1_RECT_F& bounds,
                                 const std::wstring& label, bool hovered) noexcept;

    struct QuickAction {
        std::wstring label;
        std::function<void()> callback;
    };

    DiagnosticSummary m_summary{};
    std::vector<QuickAction> m_quickActions;
    int m_hoveredActionIdx{ -1 };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };

    struct Layout {
        Input::Bounds headerArea{};
        Input::Bounds gaugesArea{};
        Input::Bounds actionsArea{};
        Input::Bounds summaryArea{};
        Input::Bounds statusBarArea{};
        float padding{ 8.0f };
        float sectionSpacing{ 6.0f };
    };
    Layout m_layout{};

    WindowManager::DragonWindow* m_pWindow{ nullptr };
    Theme::ThemeManager* m_pTheme{ nullptr };
    Input::MouseManager* m_pMouse{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
