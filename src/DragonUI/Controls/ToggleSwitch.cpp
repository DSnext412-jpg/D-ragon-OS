#include <DragonUI/Controls/ToggleSwitch.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <Windows.h>
#include <algorithm>
#include <cmath>

static float GetFrameTime() noexcept
{
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
    }();
    static LARGE_INTEGER last = [] {
        LARGE_INTEGER l;
        QueryPerformanceCounter(&l);
        return l;
    }();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float dt = static_cast<float>(now.QuadPart - last.QuadPart) / static_cast<float>(freq.QuadPart);
    last = now;
    return dt;
}

static float Lerp(float a, float b, float t) noexcept
{
    return a + (b - a) * t;
}

namespace DragonOS::DragonUI {

UIToggleSwitch::UIToggleSwitch(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
}

void UIToggleSwitch::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

void UIToggleSwitch::SetToggled(bool toggled) noexcept
{
    if (m_toggled == toggled) return;
    m_toggled = toggled;
    m_animTarget = m_toggled ? TrackWidth - ThumbSize - ThumbPadding * 2.0f : 0.0f;
    InvalidateVisual();
    if (m_onToggled) m_onToggled(*this, m_toggled);
}

void UIToggleSwitch::Toggle() noexcept
{
    SetToggled(!m_toggled);
}

// ── Measure / Render ─────────────────────────────────────────────────

DesiredSize UIToggleSwitch::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f;
    float totalW = TrackWidth + Spacing + textW;
    return {std::min(totalW, available.width), std::max(TrackHeight + 4.0f, 26.0f)};
}

void UIToggleSwitch::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    bool isDisabled = GetControlState() == ControlState::Disabled;
    bool isFocused = GetControlState() == ControlState::Focused;

    float centerY = d2d.top + slot.height * 0.5f;

    // Track rect
    D2D1_RECT_F trackRect = {
        d2d.left, centerY - TrackHeight * 0.5f,
        d2d.left + TrackWidth, centerY + TrackHeight * 0.5f
    };

    // Smooth animation toward target
    float dt = GetFrameTime();
    float speed = 10.0f;
    float diff = m_animTarget - m_thumbOffset;
    if (std::abs(diff) > 0.5f)
        m_thumbOffset += diff * std::min(dt * speed, 1.0f);
    else
        m_thumbOffset = m_animTarget;

    // Track background - lerp between off/on colors
    float t = m_thumbOffset / (TrackWidth - ThumbSize - ThumbPadding * 2.0f);
    if (TrackWidth - ThumbSize - ThumbPadding * 2.0f <= 0) t = m_toggled ? 1.0f : 0.0f;

    auto offColor = ctx.Resolve(isDisabled ? Theme::SemanticColor::Disabled : Theme::SemanticColor::ControlBorder);
    auto onColor = ctx.Resolve(isDisabled ? Theme::SemanticColor::Disabled : Theme::SemanticColor::ControlAccentFill);

    Graphics::Color trackColor = {
        offColor.r + (onColor.r - offColor.r) * t,
        offColor.g + (onColor.g - offColor.g) * t,
        offColor.b + (onColor.b - offColor.b) * t,
        1.0f
    };

    auto* rt = ctx.Renderer().GetRenderTarget();
    if (rt)
    {
        auto trackBrush = ctx.Renderer().GetBrush(trackColor);
        if (trackBrush)
        {
            D2D1_ROUNDED_RECT trackRR = {
                trackRect, CornerRadius, CornerRadius
            };
            rt->FillRoundedRectangle(&trackRR, trackBrush);
        }

        // Thumb
        float thumbX = trackRect.left + ThumbPadding + m_thumbOffset;
        float thumbY = trackRect.top + ThumbPadding;

        D2D1_RECT_F thumbRect = {
            thumbX, thumbY,
            thumbX + ThumbSize, thumbY + ThumbSize
        };

        auto thumbBrush = ctx.Renderer().GetBrush(
            isDisabled ? ctx.Resolve(Theme::SemanticColor::Disabled)
            : Graphics::Color{1.0f, 1.0f, 1.0f, 1.0f});
        if (thumbBrush)
        {
            D2D1_ROUNDED_RECT thumbRR = {
                thumbRect, ThumbCornerRadius, ThumbCornerRadius
            };
            rt->FillRoundedRectangle(&thumbRR, thumbBrush);
        }

        // Focus indicator
        if (isFocused)
        {
            D2D1_RECT_F focusRect = {
                trackRect.left - 2, trackRect.top - 2,
                trackRect.right + 2, trackRect.bottom + 2
            };
            auto focusBrush = ctx.Renderer().GetBrush(ctx.Resolve(Theme::SemanticColor::ControlAccentBorder));
            if (focusBrush)
            {
                D2D1_ROUNDED_RECT focusRR = {
                    focusRect, CornerRadius + 2.0f, CornerRadius + 2.0f
                };
                rt->DrawRoundedRectangle(&focusRR, focusBrush, 1.5f);
            }
        }
    }

    // Label text
    if (!m_text.empty())
    {
        auto textColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::TextPrimary;
        D2D1_RECT_F textRect = {
            trackRect.right + Spacing, d2d.top,
            d2d.right, d2d.bottom
        };
        ctx.DrawText(m_text, textRect, textColor);
    }
}

// ── Events ────────────────────────────────────────────────────────────

bool UIToggleSwitch::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::Click)
    {
        if (IsEnabled() && GetControlState() != ControlState::Disabled)
        {
            Toggle();
            return true;
        }
    }
    return false;
}

bool UIToggleSwitch::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::KeyDown &&
        (args.key == Input::KeyCode::Space || args.key == Input::KeyCode::Return))
    {
        if (IsEnabled())
        {
            Toggle();
            return true;
        }
    }
    return false;
}

} // namespace
