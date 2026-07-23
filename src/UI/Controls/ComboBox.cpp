#include <UI/Controls/ComboBox.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

ComboBox::ComboBox() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultComboBox());
    SetAccessibilityRole(L"combobox");
}

void ComboBox::SetItems(const std::vector<std::wstring>& items) noexcept
{
    m_items = items;
    if (m_selectedIndex >= static_cast<int>(m_items.size()))
        m_selectedIndex = m_items.empty() ? -1 : 0;
    InvalidateLayout();
}

void ComboBox::SetSelectedIndex(int index) noexcept
{
    if (index < -1 || index >= static_cast<int>(m_items.size()))
        return;
    if (m_selectedIndex != index)
    {
        m_selectedIndex = index;
        if (m_onSelectionChanged)
            m_onSelectionChanged(m_selectedIndex);
        InvalidateVisual();
    }
}

std::wstring ComboBox::GetSelectedItem() const noexcept
{
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size()))
        return m_items[m_selectedIndex];
    return {};
}

void ComboBox::SetOpen(bool open) noexcept
{
    if (m_isOpen != open)
    {
        m_isOpen = open;
        m_hoveredIndex = -1;
        if (m_isOpen)
            SetState(ElementState::Pressed);
        else
            SetState(ElementState::Normal);
        InvalidateLayout();
    }
}

D2D1_RECT_F ComboBox::GetHeaderRect() const noexcept
{
    return GetBounds();
}

D2D1_RECT_F ComboBox::GetPopupRect() const noexcept
{
    auto bounds = GetBounds();
    float itemHeight = 24.0f;
    float popupHeight = (std::min)(static_cast<float>(m_items.size()) * itemHeight, MaxPopupHeight);
    return {bounds.left, bounds.bottom, bounds.right, bounds.bottom + popupHeight};
}

D2D1_RECT_F ComboBox::GetItemRect(int index) const noexcept
{
    auto popup = GetPopupRect();
    float itemHeight = 24.0f;
    return {popup.left, popup.top + index * itemHeight, popup.right, popup.top + (index + 1) * itemHeight};
}

D2D1_RECT_F ComboBox::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float paddingL = style ? style->paddingLeft : 8.0f;
    float paddingR = style ? style->paddingRight : 8.0f;
    float paddingT = style ? style->paddingTop : 4.0f;
    float paddingB = style ? style->paddingBottom : 4.0f;

    float maxTextWidth = 80.0f;
    for (const auto& item : m_items)
    {
        float w = static_cast<float>(item.length()) * 7.0f * (fontSize / 14.0f);
        maxTextWidth = (std::max)(maxTextWidth, w);
    }

    float arrowWidth = 24.0f;
    float totalWidth = paddingL + maxTextWidth + arrowWidth + paddingR;
    float totalHeight = (std::max)(fontSize + paddingT + paddingB, MinHeaderHeight);

    totalWidth = (std::min)(totalWidth, available.right - available.left);
    totalHeight = (std::min)(totalHeight, available.bottom - available.top);

    return {0, 0, totalWidth, totalHeight};
}

void ComboBox::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);
    if (m_isOpen)
    {
        m_popupBounds = GetPopupRect();
    }
}

void ComboBox::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();
    float cornerRadius = style->cornerRadius;

    renderer.FillBackground(bounds, stateColors, cornerRadius, opacity);
    if (style->borderThickness > 0.0f)
        renderer.DrawBorder(bounds, stateColors, style->borderThickness, cornerRadius, opacity);

    float paddingL = style->paddingLeft;
    float paddingT = style->paddingTop;
    float paddingR = style->paddingRight;

    std::wstring displayText = (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size()))
        ? m_items[m_selectedIndex] : L"";

    D2D1_RECT_F textRect = {
        bounds.left + paddingL,
        bounds.top + paddingT,
        bounds.right - paddingR - 24.0f,
        bounds.bottom - style->paddingBottom
    };

    if (!displayText.empty())
        renderer.DrawText(displayText, textRect, stateColors);

    D2D1_RECT_F arrowRect = {
        bounds.right - paddingR - 18.0f,
        bounds.top + paddingT,
        bounds.right - paddingR,
        bounds.bottom - style->paddingBottom
    };
    renderer.DrawIcon(arrowRect, L'\u25BC', stateColors, 10.0f);

    if (GetState() == ElementState::Focused)
        renderer.DrawFocusIndicator(bounds);

    if (!m_isOpen) return;

    auto popup = m_popupBounds;
    StyleStateColors popupColors;
    popupColors.background = Theme::SemanticColor::WindowBackground;
    popupColors.border = Theme::SemanticColor::WindowBorder;
    popupColors.foreground = Theme::SemanticColor::TextPrimary;

    renderer.FillBackground(popup, popupColors, 0.0f, opacity);
    renderer.DrawBorder(popup, popupColors, 1.0f, 0.0f, opacity);

    float itemHeight = 24.0f;
    for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
    {
        D2D1_RECT_F itemRect = {
            popup.left, popup.top + i * itemHeight,
            popup.right, popup.top + (i + 1) * itemHeight
        };

        if (i == m_hoveredIndex)
        {
            StyleStateColors hoverColors;
            hoverColors.background = Theme::SemanticColor::Hover;
            renderer.FillBackground(itemRect, hoverColors, 0.0f, opacity);
        }
        else if (i == m_selectedIndex)
        {
            StyleStateColors selColors;
            selColors.background = Theme::SemanticColor::Selection;
            renderer.FillBackground(itemRect, selColors, 0.0f, opacity);
        }

        D2D1_RECT_F itemTextRect = {
            itemRect.left + 6.0f, itemRect.top + 2.0f,
            itemRect.right - 6.0f, itemRect.bottom - 2.0f
        };
        renderer.DrawText(m_items[i], itemTextRect, popupColors);
    }
}

bool ComboBox::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    auto headerRect = GetHeaderRect();

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        if (!m_isOpen) SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        if (!m_isOpen) SetState(ElementState::Normal);
        return true;

    case UIEventType::MouseDown:
        if (event.x >= headerRect.left && event.x <= headerRect.right &&
            event.y >= headerRect.top && event.y <= headerRect.bottom)
        {
            SetOpen(!m_isOpen);
            return true;
        }
        if (m_isOpen)
        {
            auto popup = m_popupBounds;
            if (event.x >= popup.left && event.x <= popup.right &&
                event.y >= popup.top && event.y <= popup.bottom)
            {
                int clickedIndex = static_cast<int>((event.y - popup.top) / 24.0f);
                if (clickedIndex >= 0 && clickedIndex < static_cast<int>(m_items.size()))
                {
                    SetSelectedIndex(clickedIndex);
                    SetOpen(false);
                }
                return true;
            }
            SetOpen(false);
            return true;
        }
        return false;

    case UIEventType::MouseMove:
        if (m_isOpen)
        {
            auto popup = m_popupBounds;
            if (event.x >= popup.left && event.x <= popup.right &&
                event.y >= popup.top && event.y <= popup.bottom)
            {
                int hoverIdx = static_cast<int>((event.y - popup.top) / 24.0f);
                if (hoverIdx >= 0 && hoverIdx < static_cast<int>(m_items.size()))
                {
                    if (m_hoveredIndex != hoverIdx)
                    {
                        m_hoveredIndex = hoverIdx;
                        InvalidateVisual();
                    }
                }
            }
            else
            {
                if (m_hoveredIndex != -1)
                {
                    m_hoveredIndex = -1;
                    InvalidateVisual();
                }
            }
            return true;
        }
        return false;

    case UIEventType::KeyDown:
        if (m_isOpen)
        {
            switch (event.key)
            {
            case Input::KeyCode::Up:
            {
                int newIdx = (m_hoveredIndex > 0) ? m_hoveredIndex - 1 : static_cast<int>(m_items.size()) - 1;
                m_hoveredIndex = (std::max)(0, (std::min)(newIdx, static_cast<int>(m_items.size()) - 1));
                InvalidateVisual();
                return true;
            }
            case Input::KeyCode::Down:
            {
                int newIdx = (m_hoveredIndex < static_cast<int>(m_items.size()) - 1) ? m_hoveredIndex + 1 : 0;
                m_hoveredIndex = (std::max)(0, (std::min)(newIdx, static_cast<int>(m_items.size()) - 1));
                InvalidateVisual();
                return true;
            }
            case Input::KeyCode::Return:
                if (m_hoveredIndex >= 0)
                {
                    SetSelectedIndex(m_hoveredIndex);
                    SetOpen(false);
                }
                return true;
            case Input::KeyCode::Escape:
                SetOpen(false);
                return true;
            default:
                return false;
            }
        }
        else
        {
            if (event.key == Input::KeyCode::Down || event.key == Input::KeyCode::Return || event.key == Input::KeyCode::Space)
            {
                SetOpen(true);
                m_hoveredIndex = (std::max)(0, m_selectedIndex);
                InvalidateVisual();
                return true;
            }
        }
        return false;

    case UIEventType::FocusLost:
        if (m_isOpen) SetOpen(false);
        SetState(ElementState::Normal);
        return true;

    default:
        return false;
    }
}

} // namespace
