#include <DragonUI/Controls/StatusBar.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void UIStatusBar::SetStatusText(std::wstring_view text) noexcept
{
    m_statusText = text;
    InvalidateVisual();
}

const std::wstring& UIStatusBar::GetStatusText() const noexcept
{
    return m_statusText;
}

void UIStatusBar::SetIcon(uint32_t glyph) noexcept
{
    m_iconGlyph = glyph;
    InvalidateVisual();
}

void UIStatusBar::SetProgress(float value) noexcept
{
    m_progress = (std::max)(0.0f, (std::min)(1.0f, value));
    InvalidateVisual();
}

void UIStatusBar::ClearProgress() noexcept
{
    m_progress = -1.0f;
    InvalidateVisual();
}

void UIStatusBar::SetLeftPanelContent(std::unique_ptr<Element> content) noexcept
{
    m_leftPanel = std::move(content);
    InvalidateLayout();
}

void UIStatusBar::SetRightPanelContent(std::unique_ptr<Element> content) noexcept
{
    m_rightPanel = std::move(content);
    InvalidateLayout();
}

DesiredSize UIStatusBar::MeasureOverride(const LayoutSlot& available) noexcept
{
    return {(std::max)(MinPanelWidth * 2.0f, available.width), DefaultHeight};
}

void UIStatusBar::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float leftW = (std::max)(m_leftMinWidth, finalSlot.width * 0.6f);
    float rightW = finalSlot.width - leftW;

    if (m_leftPanel)
    {
        m_leftPanel->Measure({0, 0, leftW, finalSlot.height});
        m_leftPanel->Arrange({
            finalSlot.x + PaddingH, finalSlot.y + PaddingV,
            leftW - PaddingH * 2.0f, finalSlot.height - PaddingV * 2.0f
        });
    }

    if (m_rightPanel)
    {
        m_rightPanel->Measure({0, 0, rightW - PaddingH, finalSlot.height});
        m_rightPanel->Arrange({
            finalSlot.x + leftW, finalSlot.y + PaddingV,
            rightW - PaddingH, finalSlot.height - PaddingV * 2.0f
        });
    }
}

void UIStatusBar::Render(RenderContext& ctx) noexcept
{
    auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::StatusBarBackground);

    float x = d2d.left + PaddingH;
    float centerY = d2d.top + GetBounds().height * 0.5f;

    if (m_iconGlyph)
    {
        std::wstring_view glyphStr(reinterpret_cast<const wchar_t*>(&m_iconGlyph), 1);
        auto gSize = ctx.MeasureText(glyphStr, IconSize);
        D2D1_RECT_F gRect = {
            x, centerY - gSize.height * 0.5f,
            x + gSize.width, centerY + gSize.height * 0.5f
        };
        ctx.DrawText(glyphStr, gRect, Theme::SemanticColor::StatusBarText);
        x += IconSize + 4.0f;
    }

    if (!m_statusText.empty())
    {
        float maxTextW = GetBounds().width - (x - d2d.left) - PaddingH;
        if (m_progress >= 0.0f) maxTextW -= ProgressWidth + PaddingH;

        auto textSize = ctx.MeasureText(m_statusText, maxTextW);
        D2D1_RECT_F textRect = {
            x, centerY - textSize.height * 0.5f,
            x + textSize.width, centerY + textSize.height * 0.5f
        };
        ctx.DrawText(m_statusText, textRect, Theme::SemanticColor::StatusBarText);
        x += textSize.width + PaddingH;
    }

    if (m_progress >= 0.0f)
    {
        float px = GetBounds().x + GetBounds().width - PaddingH - ProgressWidth;
        float py = centerY - ProgressHeight * 0.5f;

        D2D1_RECT_F trackRect = {
            px, py,
            px + ProgressWidth, py + ProgressHeight
        };
        ctx.FillRoundedRect(trackRect, Theme::SemanticColor::StatusBarProgressTrack,
                             ProgressHeight * 0.5f, ProgressHeight * 0.5f);

        float fillW = ProgressWidth * m_progress;
        if (fillW > 2.0f)
        {
            D2D1_RECT_F fillRect = {
                px, py,
                px + fillW, py + ProgressHeight
            };
            ctx.FillRoundedRect(fillRect, Theme::SemanticColor::StatusBarProgress,
                                 ProgressHeight * 0.5f, ProgressHeight * 0.5f);
        }
    }

    if (m_leftPanel) m_leftPanel->Render(ctx);
    if (m_rightPanel) m_rightPanel->Render(ctx);
}

} // namespace
