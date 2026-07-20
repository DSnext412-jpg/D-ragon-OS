/**
 * @file    DragonWindow.cpp
 * @brief   Implementation of the DragonWindow class.
 */

#include <WindowManager/DragonWindow.hpp>
#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>

#include <d2d1.h>

namespace DragonOS::WindowManager {

// ============================================================================
//  Construction
// ============================================================================

DragonWindow::DragonWindow(
    std::wstring  title,
    float         x,
    float         y,
    float         w,
    float         h) noexcept
    : m_title{ std::move(title) }
    , m_x{ x }
    , m_y{ y }
    , m_width{ w }
    , m_height{ h }
{
}

// ============================================================================
//  Lifecycle
// ============================================================================

void DragonWindow::Open() noexcept
{
    m_visible = true;
    m_state   = WindowState::Normal;
}

void DragonWindow::Close() noexcept
{
    m_visible = false;
    m_state   = WindowState::Hidden;
}

void DragonWindow::Minimize() noexcept
{
    m_state = WindowState::Minimized;
    m_visible = false;
}

void DragonWindow::Maximize() noexcept
{
    m_state   = WindowState::Maximized;
    // Actual viewport-filling is done by the WindowManager on resize.
}

void DragonWindow::Restore() noexcept
{
    m_state   = WindowState::Normal;
    m_visible = true;
}

// ============================================================================
//  Geometry
// ============================================================================

void DragonWindow::Move(float x, float y) noexcept
{
    m_x = x;
    m_y = y;
}

void DragonWindow::Resize(float w, float h) noexcept
{
    m_width  = w;
    m_height = h;
}

// ============================================================================
//  Focus
// ============================================================================

void DragonWindow::Focus() noexcept
{
    m_focused = true;
}

void DragonWindow::FocusLost() noexcept
{
    m_focused = false;
    m_hovered = false;
    m_pressed = false;
}

// ============================================================================
//  Hit testing
// ============================================================================

Input::HitTestRegion DragonWindow::HitTest(float px, float py) const noexcept
{
    if (!m_visible || m_state == WindowState::Hidden || m_state == WindowState::Minimized)
    {
        return Input::HitTestRegion::None;
    }

    const auto bounds = GetBounds();
    if (!bounds.Contains(px, py)) { return Input::HitTestRegion::None; }

    using TM = Theme::ThemeMetrics;

    const float titleBarBottom = m_y + TM::TitleBarHeight;
    const float borderWidth    = 4.0f;

    // ── Title bar controls (only if closable) ────────────────────────────
    if (py >= m_y && py < titleBarBottom)
    {
        const float btnWidth  = 46.0f;

        // Close button (right-most)
        if (px >= m_x + m_width - btnWidth && px < m_x + m_width)
        {
            return Input::HitTestRegion::CloseButton;
        }

        // Maximize button (only if resizable)
        if (HasFlag(m_style, WindowStyle::Resizable))
        {
            if (px >= m_x + m_width - 2.0f * btnWidth && px < m_x + m_width - btnWidth)
            {
                return Input::HitTestRegion::MaximizeButton;
            }
        }

        // Minimize button (only if closable)
        if (HasFlag(m_style, WindowStyle::Closable))
        {
            if (px >= m_x + m_width - 3.0f * btnWidth && px < m_x + m_width - 2.0f * btnWidth)
            {
                return Input::HitTestRegion::MinimizeButton;
            }
        }

        // Title bar (draggable area, only if movable)
        if (HasFlag(m_style, WindowStyle::Movable))
        {
            return Input::HitTestRegion::TitleBar;
        }

        return Input::HitTestRegion::Client;
    }

    // ── Resize borders (only if resizable) ───────────────────────────────
    if (HasFlag(m_style, WindowStyle::Resizable))
    {
        const bool onLeft  = (px - m_x) < borderWidth;
        const bool onRight = (m_x + m_width - px) < borderWidth;
        const bool onTop   = (py - m_y) < borderWidth;
        const bool onBottom = (m_y + m_height - py) < borderWidth;

        if (onTop && onLeft)     return Input::HitTestRegion::BorderTopLeft;
        if (onTop && onRight)    return Input::HitTestRegion::BorderTopRight;
        if (onBottom && onLeft)  return Input::HitTestRegion::BorderBottomLeft;
        if (onBottom && onRight) return Input::HitTestRegion::BorderBottomRight;
        if (onLeft)              return Input::HitTestRegion::BorderLeft;
        if (onRight)             return Input::HitTestRegion::BorderRight;
        if (onTop)               return Input::HitTestRegion::BorderTop;
        if (onBottom)            return Input::HitTestRegion::BorderBottom;
    }

    return Input::HitTestRegion::Client;
}

// ============================================================================
//  Rendering
// ============================================================================

void DragonWindow::SetThemeManager(const Theme::ThemeManager& themeManager) noexcept
{
    m_pThemeManager = &themeManager;
}

/// @brief  Convert a Theme::ThemeColor to Graphics::Color (identical layout).
static Graphics::Color ToGfxColor(const Theme::ThemeColor& tc) noexcept
{
    return { tc.r, tc.g, tc.b, tc.a };
}

void DragonWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_visible || m_state == WindowState::Hidden) { return; }

    auto* target = renderer.GetRenderTarget();
    if (!target) { return; }

    // ── Resolve metrics (all static constexpr — use class scope directly) ─
    using TM = Theme::ThemeMetrics;

    const float cornerRadius = TM::WindowCornerRadius;
    const float shadowOffset = TM::WindowShadowOffset;

    // ── Resolve colours from ThemeManager ────────────────────────────────
    const auto resolve = [&](Theme::SemanticColor token) -> Graphics::Color
    {
        if (m_pThemeManager)
            return ToGfxColor(m_pThemeManager->GetColor(token));

        // Static fallbacks matching the DragonOS Dark default.
        switch (token)
        {
        case Theme::SemanticColor::WindowBackground: return { 32.0f / 255.0f, 36.0f / 255.0f, 46.0f / 255.0f };
        case Theme::SemanticColor::WindowBorder:     return { 70.0f / 255.0f, 80.0f / 255.0f, 100.0f / 255.0f };
        case Theme::SemanticColor::WindowTitle:       return { 1.0f, 1.0f, 1.0f };
        case Theme::SemanticColor::Accent:            return { 58.0f / 255.0f, 134.0f / 255.0f, 255.0f / 255.0f };
        default:                                      return {};
        }
    };

    // ── Window geometry ───────────────────────────────────────────────────
    const D2D1_RECT_F clientRect = D2D1::RectF(
        m_x, m_y, m_x + m_width, m_y + m_height);

    const D2D1_ROUNDED_RECT bgRounded = D2D1::RoundedRect(
        clientRect, cornerRadius, cornerRadius);

    // ── Shadow ────────────────────────────────────────────────────────────
    {
        const D2D1_RECT_F shadowRect = D2D1::RectF(
            m_x + shadowOffset, m_y + shadowOffset,
            m_x + m_width + shadowOffset, m_y + m_height + shadowOffset);

        const D2D1_ROUNDED_RECT shadowRounded = D2D1::RoundedRect(
            shadowRect, cornerRadius, cornerRadius);

        const auto shadowColor = m_pThemeManager
            ? ToGfxColor(m_pThemeManager->GetShadow().windowShadow.color)
            : Graphics::Color{ 0.0f, 0.0f, 0.0f, 0.35f };

        auto* shadowBrush = renderer.GetBrush(shadowColor);
        if (shadowBrush)
        {
            target->FillRoundedRectangle(&shadowRounded, shadowBrush);
        }
    }

    // ── Background ────────────────────────────────────────────────────────
    {
        auto* bgBrush = renderer.GetBrush(resolve(Theme::SemanticColor::WindowBackground));
        if (bgBrush)
        {
            target->FillRoundedRectangle(&bgRounded, bgBrush);
        }
    }

    // ── Border ────────────────────────────────────────────────────────────
    {
        auto* borderBrush = renderer.GetBrush(resolve(Theme::SemanticColor::WindowBorder));
        if (borderBrush)
        {
            target->DrawRoundedRectangle(&bgRounded, borderBrush, TM::WindowBorderThickness);
        }
    }

    // ── Title bar background ──────────────────────────────────────────────
    {
        const D2D1_RECT_F titleBarRect = D2D1::RectF(
            m_x, m_y,
            m_x + m_width, m_y + TM::TitleBarHeight);

        const auto titleBarColor = m_focused
            ? resolve(Theme::SemanticColor::WindowTitleBar)
            : Graphics::Color{ 25.0f / 255.0f, 28.0f / 255.0f, 36.0f / 255.0f };

        auto* tbBrush = renderer.GetBrush(titleBarColor);
        if (tbBrush)
        {
            target->FillRectangle(&titleBarRect, tbBrush);
        }
    }

    // ── Title text ────────────────────────────────────────────────────────
    {
        // Push title rect left of control buttons
        const float controlAreaWidth = 46.0f * 3.0f;
        const D2D1_RECT_F titleRect = D2D1::RectF(
            m_x + TM::TitleTextPaddingX,
            m_y + TM::TitleTextPaddingY,
            m_x + m_width - controlAreaWidth,
            m_y + TM::TitleBarHeight);

        renderer.DrawText(m_title, titleRect, resolve(Theme::SemanticColor::WindowTitle));
    }

    // ── Control buttons ───────────────────────────────────────────────────
    {
        const float btnWidth  = 46.0f;
        const float btnHeight = TM::TitleBarHeight;
        const float rightEdge = m_x + m_width;

        // ── Close button ──────────────────────────────────────────────────
        {
            const D2D1_RECT_F btnRect = D2D1::RectF(
                rightEdge - btnWidth, m_y, rightEdge, m_y + btnHeight);

            const bool hovered  = (m_activeControlRegion == Input::HitTestRegion::CloseButton);
            const bool pressed  = (m_pressedControlRegion == Input::HitTestRegion::CloseButton);

            if (hovered || pressed)
            {
                const auto bgColor = pressed
                    ? Graphics::Color{ 1.0f, 0.2f, 0.2f, 0.8f }
                    : Graphics::Color{ 1.0f, 0.3f, 0.3f, 0.6f };
                auto* bgBrush = renderer.GetBrush(bgColor);
                if (bgBrush) { target->FillRectangle(&btnRect, bgBrush); }
            }

            // X icon
            const float pad = 12.0f;
            const D2D1_POINT_2F x1 = { btnRect.left + pad, btnRect.top + pad };
            const D2D1_POINT_2F x2 = { btnRect.right - pad, btnRect.bottom - pad };
            const D2D1_POINT_2F x3 = { btnRect.right - pad, btnRect.top + pad };
            const D2D1_POINT_2F x4 = { btnRect.left + pad, btnRect.bottom - pad };

            const auto iconColor = hovered || pressed
                ? Graphics::Color{ 1.0f, 1.0f, 1.0f }
                : Graphics::Color{ 0.8f, 0.8f, 0.8f };

            renderer.DrawLine(x1, x2, iconColor, 1.5f);
            renderer.DrawLine(x3, x4, iconColor, 1.5f);
        }

        // ── Maximize / Restore button ──────────────────────────────────────
        if (HasFlag(m_style, WindowStyle::Resizable))
        {
            const D2D1_RECT_F btnRect = D2D1::RectF(
                rightEdge - 2.0f * btnWidth, m_y, rightEdge - btnWidth, m_y + btnHeight);

            const bool hovered  = (m_activeControlRegion == Input::HitTestRegion::MaximizeButton);
            const bool pressed  = (m_pressedControlRegion == Input::HitTestRegion::MaximizeButton);

            if (hovered || pressed)
            {
                const auto bgColor = pressed
                    ? Graphics::Color{ 0.3f, 0.3f, 0.3f, 0.6f }
                    : Graphics::Color{ 0.4f, 0.4f, 0.4f, 0.4f };
                auto* bgBrush = renderer.GetBrush(bgColor);
                if (bgBrush) { target->FillRectangle(&btnRect, bgBrush); }
            }

            const auto iconColor = hovered || pressed
                ? Graphics::Color{ 1.0f, 1.0f, 1.0f }
                : Graphics::Color{ 0.8f, 0.8f, 0.8f };

            const float iconPad  = 11.0f;
            const float iconOff  = (m_state == WindowState::Maximized) ? 2.0f : 0.0f;

            if (m_state == WindowState::Maximized)
            {
                // Restore icon: two overlapping rectangles
                const D2D1_RECT_F outer = D2D1::RectF(
                    btnRect.left + iconPad + iconOff,
                    btnRect.top + iconPad,
                    btnRect.right - iconPad + iconOff,
                    btnRect.bottom - iconPad);
                const D2D1_RECT_F inner = D2D1::RectF(
                    btnRect.left + iconPad,
                    btnRect.top + iconPad - iconOff,
                    btnRect.right - iconPad,
                    btnRect.bottom - iconPad - iconOff);

                renderer.DrawRectangle(outer, iconColor, 1.5f);
                renderer.DrawRectangle(inner, iconColor, 1.5f);
            }
            else
            {
                // Maximize icon: single rectangle
                const D2D1_RECT_F iconRect = D2D1::RectF(
                    btnRect.left + iconPad, btnRect.top + iconPad,
                    btnRect.right - iconPad, btnRect.bottom - iconPad);
                renderer.DrawRectangle(iconRect, iconColor, 1.5f);
            }
        }

        // ── Minimize button ────────────────────────────────────────────────
        if (HasFlag(m_style, WindowStyle::Closable))
        {
            const D2D1_RECT_F btnRect = D2D1::RectF(
                rightEdge - 3.0f * btnWidth, m_y, rightEdge - 2.0f * btnWidth, m_y + btnHeight);

            const bool hovered  = (m_activeControlRegion == Input::HitTestRegion::MinimizeButton);
            const bool pressed  = (m_pressedControlRegion == Input::HitTestRegion::MinimizeButton);

            if (hovered || pressed)
            {
                const auto bgColor = pressed
                    ? Graphics::Color{ 0.3f, 0.3f, 0.3f, 0.6f }
                    : Graphics::Color{ 0.4f, 0.4f, 0.4f, 0.4f };
                auto* bgBrush = renderer.GetBrush(bgColor);
                if (bgBrush) { target->FillRectangle(&btnRect, bgBrush); }
            }

            const auto iconColor = hovered || pressed
                ? Graphics::Color{ 1.0f, 1.0f, 1.0f }
                : Graphics::Color{ 0.8f, 0.8f, 0.8f };

            // Minimize icon: horizontal line
            const float iconPad = 11.0f;
            const float lineY   = btnRect.top + (btnRect.bottom - btnRect.top) * 0.5f;
            renderer.DrawLine(
                { btnRect.left + iconPad, lineY },
                { btnRect.right - iconPad, lineY },
                iconColor, 1.5f);
        }
    }
}

// ============================================================================
//  Update
// ============================================================================

void DragonWindow::Update(float /*deltaTime*/) noexcept
{
    // Per-frame animation state is driven by AnimationManager;
    // this method is reserved for future per-window logic.
}

} // namespace DragonOS::WindowManager
