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

    // ── Resolve metrics from ThemeManager (fallback static defaults) ─────
    const ThemeMetrics metrics = m_pThemeManager
        ? m_pThemeManager->GetMetrics()
        : ThemeMetrics{};

    const float cornerRadius = metrics.WindowCornerRadius;
    const float shadowOffset = metrics.WindowShadowOffset;

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
            target->DrawRoundedRectangle(&bgRounded, borderBrush, metrics.WindowBorderThickness);
        }
    }

    // ── Title text ────────────────────────────────────────────────────────
    {
        const D2D1_RECT_F titleRect = D2D1::RectF(
            m_x + metrics.TitleTextPaddingX,
            m_y + metrics.TitleTextPaddingY,
            m_x + m_width - metrics.TitleTextPaddingX,
            m_y + metrics.TitleBarHeight);

        renderer.DrawText(m_title, titleRect, resolve(Theme::SemanticColor::WindowTitle));
    }
}

// ============================================================================
//  Update
// ============================================================================

void DragonWindow::Update(float deltaTime) noexcept
{
    // Reserved for future animations (fade, slide, minimise, etc.).
    (void)deltaTime;
}

} // namespace DragonOS::WindowManager
