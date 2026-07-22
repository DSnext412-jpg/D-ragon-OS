#pragma once

#include <Notifications/NotificationManager.hpp>

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Graphics   { class Renderer; }
namespace DragonOS::Theme      { class ThemeManager; }
namespace DragonOS::Input      { class MouseManager; }

namespace DragonOS::Notifications {

class NotificationCenter final {
public:
    NotificationCenter() noexcept = default;
    ~NotificationCenter() noexcept = default;

    NotificationCenter(const NotificationCenter&) = delete;
    NotificationCenter& operator=(const NotificationCenter&) = delete;
    NotificationCenter(NotificationCenter&&) = delete;
    NotificationCenter& operator=(NotificationCenter&&) = delete;

    bool Initialize(
        Theme::ThemeManager& themeMgr,
        Input::MouseManager& mouseMgr) noexcept;

    void Shutdown() noexcept;

    void Open() noexcept;
    void Close() noexcept;
    void Toggle() noexcept;
    bool IsOpen() const noexcept { return m_isOpen; }

    void Render(Graphics::Renderer& renderer) noexcept;
    void Update(float deltaTime) noexcept;
    void Resize(float viewportWidth, float viewportHeight) noexcept;
    void ProcessInput() noexcept;

    void SetNotificationManager(NotificationManager& mgr) noexcept { m_pNotifMgr = &mgr; }

    Input::Bounds GetBounds() const noexcept { return m_bounds; }

private:
    struct Layout {
        Input::Bounds panel{};
        Input::Bounds headerArea{};
        Input::Bounds dismissAllBtn{};
        Input::Bounds clearHistoryBtn{};
        float contentScrollOffset{ 0.0f };
        float contentHeight{ 0.0f };
    };

    struct NotificationLayout {
        uint64_t notificationId{ 0 };
        Input::Bounds bounds{};
        Input::Bounds dismissBtn{};
        bool isHovered{ false };
    };

    Layout CalculateLayout() const noexcept;

    void RenderPanel(Graphics::Renderer& renderer) noexcept;
    void RenderHeader(Graphics::Renderer& renderer) noexcept;
    void RenderNotifications(Graphics::Renderer& renderer) noexcept;
    void RenderSingleNotification(
        Graphics::Renderer& renderer,
        const Notification& notif,
        const NotificationLayout& nLayout) noexcept;

    Layout m_layout{};
    Input::Bounds m_bounds{};
    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
    bool m_isOpen{ false };
    float m_animProgress{ 0.0f };
    float m_animVelocity{ 0.0f };

    std::vector<NotificationLayout> m_notificationLayouts;

    Theme::ThemeManager*  m_pThemeMgr{ nullptr };
    Input::MouseManager*  m_pMouse{ nullptr };
    NotificationManager*  m_pNotifMgr{ nullptr };
    bool m_initialized{ false };
};

} // namespace DragonOS::Notifications
