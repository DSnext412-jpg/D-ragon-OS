#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <algorithm>

namespace DragonOS::UI {

void UIContainer::AddChild(std::unique_ptr<UIElement> child) noexcept
{
    if (!child)
        return;

    child->SetParent(this);
    m_children.push_back(std::move(child));
    InvalidateLayout();
    InvalidateVisual();
}

bool UIContainer::RemoveChild(UIElement* child) noexcept
{
    if (!child)
        return false;

    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const std::unique_ptr<UIElement>& ptr) { return ptr.get() == child; });

    if (it != m_children.end())
    {
        (*it)->SetParent(nullptr);
        m_children.erase(it);
        InvalidateLayout();
        InvalidateVisual();
        return true;
    }

    return false;
}

void UIContainer::ClearChildren() noexcept
{
    for (auto& child : m_children)
    {
        child->SetParent(nullptr);
    }
    m_children.clear();
    InvalidateLayout();
    InvalidateVisual();
}

UIElement* UIContainer::GetChildAt(size_t index) const noexcept
{
    if (index < m_children.size())
        return m_children[index].get();

    return nullptr;
}

void UIContainer::Measure(const D2D1_RECT_F& availableSize) noexcept
{
    if (m_visibility != Visibility::Visible)
    {
        m_desiredSize = {0, 0, 0, 0};
        return;
    }

    float availW = availableSize.right - availableSize.left;
    float availH = availableSize.bottom - availableSize.top;

    D2D1_RECT_F contentAvailable = {
        0, 0,
        (std::max)(0.0f, availW - m_margin[0] - m_margin[2]),
        (std::max)(0.0f, availH - m_margin[1] - m_margin[3])
    };

    D2D1_RECT_F desired = MeasureOverride(contentAvailable);

    float childrenW = 0, childrenH = 0;
    if (m_visibility == Visibility::Visible)
    {
        D2D1_RECT_F childAvailable = {
            0, 0,
            (std::max)(0.0f, contentAvailable.right - m_padding[0] - m_padding[2]),
            (std::max)(0.0f, contentAvailable.bottom - m_padding[1] - m_padding[3])
        };

        D2D1_RECT_F childrenDesired = MeasureChildren(childAvailable);
        childrenW = childrenDesired.right - childrenDesired.left;
        childrenH = childrenDesired.bottom - childrenDesired.top;
    }

    float desiredW = (std::max)(desired.right - desired.left, childrenW);
    float desiredH = (std::max)(desired.bottom - desired.top, childrenH);

    desiredW = (std::clamp)(desiredW, m_minWidth, m_maxWidth);
    desiredH = (std::clamp)(desiredH, m_minHeight, m_maxHeight);

    m_desiredSize = {
        0, 0,
        desiredW + m_margin[0] + m_margin[2],
        desiredH + m_margin[1] + m_margin[3]
    };

    m_layoutDirty = false;
}

void UIContainer::Arrange(const D2D1_RECT_F& finalRect) noexcept
{
    m_bounds = finalRect;

    D2D1_RECT_F contentRect = {
        finalRect.left + m_padding[0],
        finalRect.top + m_padding[1],
        (std::max)(finalRect.left + m_padding[0], finalRect.right - m_padding[2]),
        (std::max)(finalRect.top + m_padding[1], finalRect.bottom - m_padding[3])
    };

    ArrangeOverride(contentRect);
    ArrangeChildren(contentRect);

    m_layoutDirty = false;
}

void UIContainer::Render(UIRenderer& renderer) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    if (m_clipChildren)
    {
        renderer.PushClip(m_bounds);
    }

    SortChildrenByZIndex();

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
        {
            child->Render(renderer);
        }
    }

    if (m_clipChildren)
    {
        renderer.PopClip();
    }
}

UIElement* UIContainer::HitTest(float x, float y) noexcept
{
    if (m_visibility != Visibility::Visible || !m_enabled)
        return nullptr;

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
        UIElement* hit = (*it)->HitTest(x, y);
        if (hit)
            return hit;
    }

    if (x >= m_bounds.left && x <= m_bounds.right &&
        y >= m_bounds.top && y <= m_bounds.bottom)
    {
        return this;
    }

    return nullptr;
}

bool UIContainer::OnEvent(const UIEvent& event) noexcept
{
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
        if ((*it)->OnEvent(event))
            return true;
    }

    return UIElement::OnEvent(event);
}

void UIContainer::SortChildrenByZIndex() noexcept
{
    std::stable_sort(m_children.begin(), m_children.end(),
        [](const std::unique_ptr<UIElement>& a, const std::unique_ptr<UIElement>& b)
        {
            return a->GetZIndex() < b->GetZIndex();
        });
}

D2D1_RECT_F UIContainer::MeasureChildren(const D2D1_RECT_F& availableSize) noexcept
{
    float maxW = 0, maxH = 0;

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Visible)
            continue;

        child->Measure(availableSize);

        const auto& ds = child->GetDesiredSize();
        float childW = ds.right - ds.left;
        float childH = ds.bottom - ds.top;

        if (childW > maxW) maxW = childW;
        if (childH > maxH) maxH = childH;
    }

    return {0, 0, maxW, maxH};
}

void UIContainer::ArrangeChildren(const D2D1_RECT_F& finalRect) noexcept
{
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
        {
            child->Arrange(finalRect);
        }
    }
}

} // namespace
