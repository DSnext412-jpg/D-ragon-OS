#include <Notifications/NotificationCenter.hpp>

#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>

#include <d2d1.h>

namespace DragonOS::Notifications {

bool NotificationCenter::Initialize(
    Theme::ThemeManager& themeMgr,
    Input::MouseManager& mouseMgr) noexcept
{
    if (m_initialized) { return true; }
    m_pThemeMgr = &themeMgr;
    m_pMouse = &mouseMgr;
    m_initialized = true;
    return true;
}

void NotificationCenter::Shutdown() noexcept
{
    if (!m_initialized) { return; }
    m_pThemeMgr = nullptr;
    m_pMouse = nullptr;
    m_initialized = false;
}

void NotificationCenter::Open() noexcept
{
    if (!m_initialized) { return; }
    m_isOpen = true;
    m_animVelocity = 0.0f;
}

void NotificationCenter::Close() noexcept
{
    m_isOpen = false;
    m_animVelocity = 0.0f;
}

void NotificationCenter::Toggle() noexcept
{
    if (m_isOpen) { Close(); }
    else { Open(); }
}

NotificationCenter::Layout NotificationCenter::CalculateLayout() const noexcept
{
    Layout lay{};

    const float panelW = 360.0f;
    const float panelH = m_viewportHeight - Theme::ThemeMetrics::TaskbarHeight;
    const float panelX = m_viewportWidth - panelW;
    const float panelY = 0.0f;

    lay.panel = { panelX, panelY, panelW, panelH };

    const float headerH = 48.0f;
    lay.headerArea = { panelX, panelY, panelW, headerH };

    const float btnW = 120.0f;
    const float btnH = 28.0f;
    lay.dismissAllBtn = { panelX + panelW - btnW - 8.0f, panelY + (headerH - btnH) * 0.5f, btnW, btnH };
    lay.clearHistoryBtn = { panelX + panelW - btnW * 2.0f - 16.0f, panelY + (headerH - btnH) * 0.5f, btnW, btnH };

    lay.contentScrollOffset = 0.0f;
    lay.contentHeight = 0.0f;

    return lay;
}

void NotificationCenter::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized || !m_isOpen) { return; }

    m_layout = CalculateLayout();
    RenderPanel(renderer);
    RenderHeader(renderer);
    RenderNotifications(renderer);
}

void NotificationCenter::RenderPanel(Graphics::Renderer& renderer) noexcept
{
    const auto& bgColor = m_pThemeMgr->GetColor(Theme::SemanticColor::NotificationBackground);
    const Graphics::Color bg{ bgColor.r, bgColor.g, bgColor.b, bgColor.a };

    const auto& p = m_layout.panel;
    const D2D1_RECT_F rect = D2D1::RectF(p.x, p.y, p.Right(), p.Bottom());
    renderer.FillRectangle(rect, bg);

    const auto& borderColor = m_pThemeMgr->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color border{ borderColor.r, borderColor.g, borderColor.b, 0.5f };
    renderer.DrawLine(
        D2D1::Point2F(p.x, p.y),
        D2D1::Point2F(p.x, p.Bottom()),
        border, 1.0f);
}

void NotificationCenter::RenderHeader(Graphics::Renderer& renderer) noexcept
{
    const auto& h = m_layout.headerArea;
    const auto& textCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a };

    const D2D1_RECT_F titleRect = D2D1::RectF(
        h.x + 12.0f, h.y + 4.0f, h.Right() - 12.0f, h.y + 28.0f);
    renderer.DrawText(L"Notifications", titleRect, tc);

    if (m_pNotifMgr && m_pNotifMgr->GetActiveCount() > 0)
    {
        const auto& accColor = m_pThemeMgr->GetColor(Theme::SemanticColor::Accent);
        const Graphics::Color ac{ accColor.r, accColor.g, accColor.b, accColor.a };

        const D2D1_RECT_F countRect = D2D1::RectF(
            h.x + 12.0f, h.y + 26.0f, h.Right() - 12.0f, h.y + h.height);
        const std::wstring countStr = std::to_wstring(m_pNotifMgr->GetActiveCount()) + L" active";
        renderer.DrawText(countStr, countRect, ac);
    }

    const auto& disCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color dc{ disCol.r, disCol.g, disCol.b, disCol.a };

    const D2D1_RECT_F dismissRect = D2D1::RectF(
        m_layout.dismissAllBtn.x, m_layout.dismissAllBtn.y,
        m_layout.dismissAllBtn.Right(), m_layout.dismissAllBtn.Bottom());
    renderer.DrawText(L"Dismiss All", dismissRect, dc);
}

void NotificationCenter::RenderNotifications(Graphics::Renderer& renderer) noexcept
{
    if (!m_pNotifMgr) { return; }

    auto active = m_pNotifMgr->GetActive();
    m_notificationLayouts.clear();

    const auto& p = m_layout.panel;
    float y = m_layout.headerArea.Bottom() + 4.0f;

    if (active.empty())
    {
        const auto& secCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextSecondary);
        const Graphics::Color sc{ secCol.r, secCol.g, secCol.b, secCol.a };

        const D2D1_RECT_F emptyRect = D2D1::RectF(
            p.x + 12.0f, y, p.Right() - 12.0f, y + 40.0f);
        renderer.DrawText(L"No new notifications", emptyRect, sc);
        return;
    }

    for (const auto* notif : active)
    {
        if (!notif) { continue; }

        const float notifH = 72.0f;
        NotificationLayout nl;
        nl.notificationId = notif->id;
        nl.bounds = { p.x + 4.0f, y, p.width - 8.0f, notifH };

        const float dBtnSize = 16.0f;
        nl.dismissBtn = {
            nl.bounds.Right() - dBtnSize - 4.0f,
            nl.bounds.y + 4.0f,
            dBtnSize, dBtnSize
        };

        m_notificationLayouts.push_back(nl);
        RenderSingleNotification(renderer, *notif, nl);

        y += notifH + 4.0f;
    }
}

void NotificationCenter::RenderSingleNotification(
    Graphics::Renderer& renderer,
    const Notification& notif,
    const NotificationLayout& nLayout) noexcept
{
    const auto& bounds = nLayout.bounds;
    const D2D1_RECT_F bgRect = D2D1::RectF(bounds.x, bounds.y, bounds.Right(), bounds.Bottom());

    if (nLayout.isHovered)
    {
        const auto& hoverCol = m_pThemeMgr->GetColor(Theme::SemanticColor::Hover);
        const Graphics::Color hc{ hoverCol.r, hoverCol.g, hoverCol.b, hoverCol.a };
        renderer.FillRectangle(bgRect, hc);
    }

    Theme::SemanticColor accentToken;
    switch (notif.severity)
    {
    case NotificationSeverity::Warning:  accentToken = Theme::SemanticColor::NotificationWarning; break;
    case NotificationSeverity::Error:    accentToken = Theme::SemanticColor::NotificationError; break;
    case NotificationSeverity::Success:  accentToken = Theme::SemanticColor::NotificationSuccess; break;
    default:                             accentToken = Theme::SemanticColor::NotificationInfo; break;
    }

    const auto& accCol = m_pThemeMgr->GetColor(accentToken);
    const Graphics::Color ac{ accCol.r, accCol.g, accCol.b, accCol.a };

    const D2D1_RECT_F accentBar = D2D1::RectF(bounds.x, bounds.y, bounds.x + 3.0f, bounds.Bottom());
    renderer.FillRectangle(accentBar, ac);

    const auto& primaryCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color pc{ primaryCol.r, primaryCol.g, primaryCol.b, primaryCol.a };

    const D2D1_RECT_F titleRect = D2D1::RectF(
        bounds.x + 10.0f, bounds.y + 4.0f, bounds.Right() - 24.0f, bounds.y + 24.0f);
    renderer.DrawText(notif.title, titleRect, pc);

    const auto& secCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color sc{ secCol.r, secCol.g, secCol.b, secCol.a };

    const D2D1_RECT_F msgRect = D2D1::RectF(
        bounds.x + 10.0f, bounds.y + 24.0f, bounds.Right() - 10.0f, bounds.Bottom() - 4.0f);
    renderer.DrawText(notif.message, msgRect, sc);

    if (notif.severity == NotificationSeverity::Progress)
    {
        const float barY = bounds.Bottom() - 6.0f;
        const float barH = 4.0f;

        const D2D1_RECT_F barBg = D2D1::RectF(
            bounds.x + 10.0f, barY, bounds.Right() - 10.0f, barY + barH);
        const Graphics::Color barBgCol{ 0.3f, 0.3f, 0.3f, 0.3f };
        renderer.FillRectangle(barBg, barBgCol);

        const float filledW = (bounds.width - 20.0f) * (std::min)(1.0f, notif.progress);
        const D2D1_RECT_F barFill = D2D1::RectF(
            bounds.x + 10.0f, barY, bounds.x + 10.0f + filledW, barY + barH);
        renderer.FillRectangle(barFill, ac);
    }

    const auto& disCol = m_pThemeMgr->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color dc{ disCol.r, disCol.g, disCol.b, disCol.a };

    const D2D1_RECT_F dBtnRect = D2D1::RectF(
        nLayout.dismissBtn.x, nLayout.dismissBtn.y,
        nLayout.dismissBtn.Right(), nLayout.dismissBtn.Bottom());
    renderer.DrawText(L"\u00D7", dBtnRect, dc);
}

void NotificationCenter::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    if (m_isOpen && m_animProgress < 1.0f)
    {
        m_animVelocity += (1.0f - m_animProgress) * 12.0f * deltaTime;
        m_animVelocity *= 0.9f;
        m_animProgress += m_animVelocity * deltaTime;
        if (m_animProgress > 1.0f) { m_animProgress = 1.0f; }
    }
    else if (!m_isOpen && m_animProgress > 0.0f)
    {
        m_animVelocity += (0.0f - m_animProgress) * 12.0f * deltaTime;
        m_animVelocity *= 0.9f;
        m_animProgress += m_animVelocity * deltaTime;
        if (m_animProgress < 0.0f) { m_animProgress = 0.0f; }
    }
}

void NotificationCenter::Resize(float viewportWidth, float viewportHeight) noexcept
{
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
}

void NotificationCenter::ProcessInput() noexcept
{
    if (!m_initialized || !m_isOpen || !m_pMouse) { return; }

    const auto pos = m_pMouse->GetPosition();

    for (auto& nl : m_notificationLayouts)
    {
        nl.isHovered = nl.bounds.Contains(pos.x, pos.y);
    }

    if (m_pMouse->WasLeftClicked())
    {
        for (const auto& nl : m_notificationLayouts)
        {
            if (nl.dismissBtn.Contains(pos.x, pos.y) && m_pNotifMgr)
            {
                m_pNotifMgr->Dismiss(nl.notificationId);
                break;
            }
        }
    }
}

} // namespace DragonOS::Notifications
