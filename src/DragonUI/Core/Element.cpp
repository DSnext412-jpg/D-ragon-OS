#include <DragonUI/Core/Element.hpp>
#include <DragonUI/Core/Container.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

uint64_t Element::s_nextId = 1;

Element::Element() noexcept
    : m_id(s_nextId++)
    , m_maxW(FLT_MAX)
    , m_maxH(FLT_MAX)
{
}

void Element::Measure(const LayoutSlot& available) noexcept
{
    if (m_visibility != Visibility::Visible)
    {
        m_desiredSize = {};
        return;
    }

    auto avail = available.Inset(m_margin);
    auto desired = MeasureOverride(avail);

    desired.width = (std::clamp)(desired.width, m_minW, m_maxW);
    desired.height = (std::clamp)(desired.height, m_minH, m_maxH);

    m_desiredSize = {
        desired.width + m_margin.Horizontal(),
        desired.height + m_margin.Vertical()
    };

    m_layoutDirty = false;
}

void Element::Arrange(const LayoutSlot& finalSlot) noexcept
{
    m_bounds = finalSlot;

    auto content = finalSlot.Inset(m_padding);
    ArrangeOverride(content);

    m_layoutDirty = false;
}

void Element::Render(RenderContext& /*ctx*/) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;
}

Element* Element::HitTest(float x, float y) noexcept
{
    if (m_visibility != Visibility::Visible || !m_enabled)
        return nullptr;

    if (x >= m_bounds.x && x <= m_bounds.x + m_bounds.width &&
        y >= m_bounds.y && y <= m_bounds.y + m_bounds.height)
    {
        return this;
    }

    return nullptr;
}

DesiredSize Element::MeasureOverride(const LayoutSlot& /*available*/) noexcept
{
    return {};
}

void Element::ArrangeOverride(const LayoutSlot& /*finalSlot*/) noexcept
{
}

} // namespace
