#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void Container::AddChild(std::unique_ptr<Element> child) noexcept
{
    if (!child) return;
    child->SetParent(this);
    m_children.push_back(std::move(child));
    InvalidateLayout();
}

bool Container::RemoveChild(Element* child) noexcept
{
    if (!child) return false;

    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const std::unique_ptr<Element>& ptr) { return ptr.get() == child; });

    if (it != m_children.end())
    {
        (*it)->SetParent(nullptr);
        m_children.erase(it);
        InvalidateLayout();
        return true;
    }
    return false;
}

void Container::ClearChildren() noexcept
{
    for (auto& child : m_children)
        child->SetParent(nullptr);
    m_children.clear();
    InvalidateLayout();
}

Element* Container::GetChildAt(size_t index) const noexcept
{
    return index < m_children.size() ? m_children[index].get() : nullptr;
}

void Container::Measure(const LayoutSlot& available) noexcept
{
    if (m_visibility != Visibility::Visible)
    {
        m_desiredSize = {};
        return;
    }

    auto avail = available.Inset(m_margin);
    auto desired = MeasureOverride(avail);

    MeasureChildren(avail.Inset(m_padding));

    float childrenW = 0, childrenH = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Visible) continue;
        auto ds = child->GetDesiredSize();
        if (ds.width > childrenW) childrenW = ds.width;
        if (ds.height > childrenH) childrenH = ds.height;
    }

    float totalW = (std::max)(desired.width, childrenW);
    float totalH = (std::max)(desired.height, childrenH);

    totalW = (std::clamp)(totalW, m_minW, m_maxW);
    totalH = (std::clamp)(totalH, m_minH, m_maxH);

    m_desiredSize = {totalW + m_margin.Horizontal(), totalH + m_margin.Vertical()};
    m_layoutDirty = false;
}

void Container::Arrange(const LayoutSlot& finalSlot) noexcept
{
    m_bounds = finalSlot;
    auto content = finalSlot.Inset(m_padding);
    ArrangeOverride(content);
    ArrangeChildren(content);
    m_layoutDirty = false;
}

void Container::Render(RenderContext& ctx) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    if (m_clipChildren)
        ctx.PushClip(static_cast<D2D1_RECT_F>(m_bounds));

    RenderChildren(ctx);

    if (m_clipChildren)
        ctx.PopClip();
}

Element* Container::HitTest(float x, float y) noexcept
{
    if (m_visibility != Visibility::Visible || !m_enabled)
        return nullptr;

    auto* hit = HitTestChildren(x, y);
    if (hit) return hit;

    if (x >= m_bounds.x && x <= m_bounds.x + m_bounds.width &&
        y >= m_bounds.y && y <= m_bounds.y + m_bounds.height)
        return this;

    return nullptr;
}

void Container::SortChildrenByZOrder() noexcept
{
    std::stable_sort(m_children.begin(), m_children.end(),
        [](const std::unique_ptr<Element>& a, const std::unique_ptr<Element>& b) {
            return a->GetZOrder() < b->GetZOrder();
        });
}

void Container::MeasureChildren(const LayoutSlot& available) noexcept
{
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
            child->Measure(available);
    }
}

void Container::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
            child->Arrange(finalSlot);
    }
}

void Container::RenderChildren(RenderContext& ctx) noexcept
{
    SortChildrenByZOrder();
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
            child->Render(ctx);
    }
}

Element* Container::HitTestChildren(float x, float y) noexcept
{
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
        auto* hit = (*it)->HitTest(x, y);
        if (hit) return hit;
    }
    return nullptr;
}

} // namespace
