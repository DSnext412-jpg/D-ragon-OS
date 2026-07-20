#pragma once

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>
#include <Theme/ThemeMetrics.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Graphics   { class Renderer; }
namespace DragonOS::Theme      { class ThemeManager; }
namespace DragonOS::Input      { class MouseManager; }
namespace DragonOS::Animation  { class AnimationManager; }
namespace DragonOS::WindowManager { class WindowManager; class DragonWindow; }
namespace DragonOS::StartMenu  { class StartMenuController; }

namespace DragonOS::Taskbar {

enum class TaskbarHitRegion : uint8_t {
    None,
    StartButton,
    TaskItem,
    TrayVolume,
    TrayNetwork,
    TrayBattery,
    TrayNotifications,
    Clock,
    StartMenu,
};

struct TaskItem {
    uint64_t      windowId{ 0 };
    std::wstring  title;
    Input::Bounds bounds{};
    bool          isActive{ false };
    bool          isHovered{ false };
    bool          isPressed{ false };
    float         hoverAnim{ 0.0f };
};

class Taskbar final {
public:
    Taskbar() noexcept;
    ~Taskbar() noexcept;

    Taskbar(const Taskbar&)            = delete;
    Taskbar& operator=(const Taskbar&) = delete;
    Taskbar(Taskbar&&)                 = delete;
    Taskbar& operator=(Taskbar&&)      = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    bool Initialize(
        Theme::ThemeManager&       themeManager,
        Input::MouseManager&       mouseManager,
        WindowManager::WindowManager& windowManager,
        Animation::AnimationManager&  animManager) noexcept;

    void Shutdown() noexcept;

    // ── Per-frame ─────────────────────────────────────────────────────────

    void Render(Graphics::Renderer& renderer) noexcept;
    void Update(float deltaTime) noexcept;
    void Resize(float viewportWidth, float viewportHeight) noexcept;

    // ── Input ─────────────────────────────────────────────────────────────

    void ProcessInput() noexcept;

    // ── Start Menu integration ────────────────────────────────────────────

    /// @brief  Attach a StartMenuController so the start button toggles it.
    void SetStartMenuController(StartMenu::StartMenuController& controller) noexcept
    {
        m_pStartMenu = &controller;
    }

    // ── Accessors ─────────────────────────────────────────────────────────

    [[nodiscard]] float GetHeight() const noexcept { return m_height; }

    [[nodiscard]] Input::Bounds GetBounds() const noexcept
    {
        return { 0.0f, m_viewportHeight - m_height,
                 m_viewportWidth, m_height };
    }

private:
    // ── Layout ────────────────────────────────────────────────────────────

    struct Layout {
        Input::Bounds bar{};
        Input::Bounds startButton{};
        Input::Bounds taskListArea{};
        Input::Bounds clockArea{};
        Input::Bounds trayArea{};

        Input::Bounds trayVolume{};
        Input::Bounds trayNetwork{};
        Input::Bounds trayBattery{};
        Input::Bounds trayNotifications{};
    };

    [[nodiscard]] Layout CalculateLayout(
        float vpW, float vpH, float height) const noexcept;

    // ── Rendering helpers ─────────────────────────────────────────────────

    void RenderBackground(Graphics::Renderer& renderer) noexcept;
    void RenderStartButton(Graphics::Renderer& renderer) noexcept;
    void RenderTaskItems(Graphics::Renderer& renderer) noexcept;
    void RenderSystemTray(Graphics::Renderer& renderer) noexcept;
    void RenderClock(Graphics::Renderer& renderer) noexcept;

    void RenderTaskItem(
        Graphics::Renderer& renderer,
        const TaskItem&     item) noexcept;

    // ── Animation helpers ─────────────────────────────────────────────────

    void UpdateAnimations(float deltaTime) noexcept;

    // ── Data ──────────────────────────────────────────────────────────────

    Layout               m_layout{};
    float                m_viewportWidth{ 0.0f };
    float                m_viewportHeight{ 0.0f };
    float                m_height{ Theme::ThemeMetrics::TaskbarHeight };
    std::uint32_t        m_lastTargetGen{ 0 };

    // ── Start button ─────────────────────────────────────────────────────

    TaskbarHitRegion     m_hoveredRegion{ TaskbarHitRegion::None };
    TaskbarHitRegion     m_pressedRegion{ TaskbarHitRegion::None };
    float                m_startBtnHoverAnim{ 0.0f };

    // ── Task list ─────────────────────────────────────────────────────────

    std::vector<TaskItem> m_taskItems;
    uint64_t              m_lastFocusedId{ 0 };

    // ── Clock ─────────────────────────────────────────────────────────────

    std::wstring          m_timeString;
    std::wstring          m_dateString;
    float                 m_clockTimer{ 0.0f };

    // ── Non-owning references ─────────────────────────────────────────────

    Theme::ThemeManager*           m_pThemeManager{ nullptr };
    Input::MouseManager*           m_pMouse{ nullptr };
    WindowManager::WindowManager*  m_pWindowManager{ nullptr };
    Animation::AnimationManager*   m_pAnimManager{ nullptr };
    StartMenu::StartMenuController* m_pStartMenu{ nullptr };

    bool                  m_initialized{ false };
};

} // namespace DragonOS::Taskbar
