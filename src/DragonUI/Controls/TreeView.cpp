#include <DragonUI/Controls/TreeView.hpp>
#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cmath>

namespace DragonOS::DragonUI {

// ── UITreeNode ────────────────────────────────────────────────────────

UITreeNode::UITreeNode(std::wstring_view text, uint32_t icon) noexcept
    : m_text(text)
    , m_iconGlyph(icon)
{
}

void UITreeNode::SetText(std::wstring_view text) noexcept
{
    m_text = text;
}

void UITreeNode::SetLazyLoader(std::function<void(UITreeNode&)> loader) noexcept
{
    m_lazyLoader = std::move(loader);
}

void UITreeNode::LoadChildrenIfNeeded() noexcept
{
    if (m_children.empty() && m_lazyLoader)
        m_lazyLoader(*this);
}

bool UITreeNode::NeedsLazyLoad() const noexcept
{
    return m_children.empty() && m_lazyLoader;
}

int UITreeNode::GetDepth() const noexcept
{
    int depth = 0;
    for (const UITreeNode* p = m_parent; p != nullptr; p = p->m_parent)
        ++depth;
    return depth;
}

UITreeNode* UITreeNode::AddChild(std::wstring_view text, uint32_t icon) noexcept
{
    auto child = std::make_unique<UITreeNode>(text, icon);
    child->m_parent = this;
    UITreeNode* raw = child.get();
    m_children.push_back(std::move(child));
    return raw;
}

void UITreeNode::ClearChildren() noexcept
{
    m_children.clear();
}

// ── UITreeView ────────────────────────────────────────────────────────

UITreeView::UITreeView() noexcept
{
    m_focusable = true;
    SetMinSize(160.0f, 120.0f);
    m_root = std::make_unique<UITreeNode>();
    m_selection.SetMode(SelectionMode::Single);
    m_selection.SetOnChanged([this](const SelectionManager& sm) { OnSelectionChanged(sm); });
}

UITreeNode* UITreeView::AddRootNode(std::wstring_view text, uint32_t icon) noexcept
{
    auto* node = m_root->AddChild(text, icon);
    m_visibleDirty = true;
    InvalidateLayout();
    return node;
}

void UITreeView::ClearNodes() noexcept
{
    m_root->ClearChildren();
    m_selection.Clear();
    m_visibleDirty = true;
    InvalidateLayout();
}

// ── Expansion ─────────────────────────────────────────────────────────

void UITreeView::ExpandNode(UITreeNode& node) noexcept
{
    if (node.m_expanded)
        return;

    node.LoadChildrenIfNeeded();
    if (!node.HasChildren())
    {
        node.m_leaf = true;
        return;
    }

    node.m_expanded = true;
    m_visibleDirty = true;
    InvalidateLayout();
    if (m_onNodeExpanded)
        m_onNodeExpanded(node, true);
}

void UITreeView::CollapseNode(UITreeNode& node) noexcept
{
    if (!node.m_expanded)
        return;

    node.m_expanded = false;
    m_visibleDirty = true;
    InvalidateLayout();
    if (m_onNodeExpanded)
        m_onNodeExpanded(node, false);
}

void UITreeView::ToggleNode(UITreeNode& node) noexcept
{
    if (node.IsExpanded())
        CollapseNode(node);
    else
        ExpandNode(node);
}

void UITreeView::ExpandAll() noexcept
{
    const auto visit = [](auto& self, UITreeNode& node) -> void {
        for (const auto& child : node.m_children)
        {
            child->LoadChildrenIfNeeded();
            child->m_expanded = true;
            self(self, *child);
        }
    };
    visit(visit, *m_root);

    m_visibleDirty = true;
    InvalidateLayout();
}

void UITreeView::CollapseAll() noexcept
{
    const auto visit = [](auto& self, UITreeNode& node) -> void {
        for (const auto& child : node.m_children)
        {
            child->m_expanded = false;
            self(self, *child);
        }
    };
    visit(visit, *m_root);

    m_visibleDirty = true;
    InvalidateLayout();
}

// ── Selection ─────────────────────────────────────────────────────────

UITreeNode* UITreeView::GetSelectedNode() const noexcept
{
    const int64_t index = m_selection.GetFirstSelected();
    return GetVisibleNode(index);
}

void UITreeView::SelectNode(UITreeNode& node) noexcept
{
    const int64_t index = GetVisibleIndex(node);
    if (index < 0)
    {
        // Node is collapsed; expand ancestors first.
        UITreeNode* p = node.GetParent();
        while (p)
        {
            p->m_expanded = true;
            p = p->GetParent();
        }
        m_visibleDirty = true;
        FlattenVisible();
        const int64_t retry = GetVisibleIndex(node);
        if (retry < 0)
            return;
        m_selection.SelectSingle(retry);
        EnsureVisible(retry);
        return;
    }
    m_selection.SelectSingle(index);
    EnsureVisible(index);
}

// ── Visible flattening ────────────────────────────────────────────────

UITreeNode* UITreeView::GetVisibleNode(int64_t index) const noexcept
{
    if (index < 0 || index >= static_cast<int64_t>(m_visible.size()))
        return nullptr;
    return m_visible[static_cast<size_t>(index)];
}

int64_t UITreeView::GetVisibleIndex(const UITreeNode& node) const noexcept
{
    for (int64_t i = 0; i < static_cast<int64_t>(m_visible.size()); ++i)
    {
        if (m_visible[static_cast<size_t>(i)] == &node)
            return i;
    }
    return -1;
}

void UITreeView::Refresh() noexcept
{
    m_visibleDirty = true;
    FlattenVisible();
    InvalidateVisual();
}

void UITreeView::FlattenVisible() noexcept
{
    if (!m_visibleDirty)
        return;

    m_visible.clear();

    const auto walk = [](auto& self, UITreeNode& node, std::vector<UITreeNode*>& out) -> void {
        for (const auto& child : node.m_children)
        {
            out.push_back(child.get());
            if (child->m_expanded)
                self(self, *child, out);
        }
    };
    walk(walk, *m_root, m_visible);

    m_selection.SetItemCount(static_cast<int64_t>(m_visible.size()));
    m_visibleDirty = false;
}

// ── Scrolling ─────────────────────────────────────────────────────────

void UITreeView::SetScrollOffset(float offset) noexcept
{
    m_scrollOffset = offset;
    InvalidateVisual();
}

void UITreeView::ScrollToNode(UITreeNode& node) noexcept
{
    const int64_t index = GetVisibleIndex(node);
    if (index < 0)
        return;
    EnsureVisible(index);
}

float UITreeView::GetContentHeight() const noexcept
{
    return static_cast<float>(m_visible.size()) * m_rowHeight;
}

// ── Hit testing ───────────────────────────────────────────────────────

UITreeNode* UITreeView::HitTestNodeAt(float x, float y) const noexcept
{
    const float localY = y - GetY();
    if (x < GetX() || x > GetX() + GetWidth())
        return nullptr;

    const int64_t row = static_cast<int64_t>((localY + m_scrollOffset) / m_rowHeight);
    if (row < 0 || row >= static_cast<int64_t>(m_visible.size()))
        return nullptr;
    return m_visible[static_cast<size_t>(row)];
}

// ── Layout ────────────────────────────────────────────────────────────

DesiredSize UITreeView::MeasureOverride(const LayoutSlot& /*available*/) noexcept
{
    return { m_minW > 0.0f ? m_minW : 200.0f, m_minH > 0.0f ? m_minH : 200.0f };
}

// ── Rendering ─────────────────────────────────────────────────────────

void UITreeView::Render(RenderContext& ctx) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;
    FlattenVisible();

    const float contentHeight = GetContentHeight();
    const float maxScroll = (std::max)(0.0f, contentHeight - GetHeight());
    m_scrollOffset = (std::clamp)(m_scrollOffset, 0.0f, maxScroll);

    const auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::ControlFill, 0.4f);
    ctx.PushClip(d2d);

    const int64_t total = static_cast<int64_t>(m_visible.size());
    if (total == 0)
    {
        ctx.DrawText(L"(empty)", { GetX() + 12.0f, GetY() + 8.0f, GetX() + 200.0f, GetY() + 28.0f },
            Theme::SemanticColor::PlaceholderText);
        ctx.PopClip();
        RenderScrollBar(ctx);
        return;
    }

    const int64_t first = static_cast<int64_t>(m_scrollOffset / m_rowHeight);
    const int64_t last = static_cast<int64_t>((m_scrollOffset + GetHeight()) / m_rowHeight);

    for (int64_t row = first; row <= (std::min)(last, total - 1); ++row)
    {
        const float top = GetY() + row * m_rowHeight - m_scrollOffset;
        const LayoutSlot slot{ GetX(), top, GetWidth() - ScrollBarWidth, m_rowHeight };
        RenderNode(ctx, slot, *m_visible[static_cast<size_t>(row)], row);
    }

    ctx.PopClip();
    RenderScrollBar(ctx);
}

void UITreeView::RenderNode(RenderContext& ctx, const LayoutSlot& slot, UITreeNode& node, int64_t index) noexcept
{
    const bool selected = m_selection.IsSelected(index);
    const bool hovered = (index == m_hoveredIndex);
    const bool focused = (GetControlState() == ControlState::Focused);

    if (selected)
        ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Selection);
    else if (hovered)
        ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover);

    const int depth = node.GetDepth() - 1; // Root level is 0.
    const float indent = static_cast<float>(depth) * m_indentWidth;
    const float cx = slot.x + indent;

    // Expand / collapse arrow.
    if (!node.IsLeaf())
    {
        const D2D1_RECT_F arrowRect = {
            cx + 2.0f, slot.y + 2.0f,
            cx + ExpandArrowWidth, slot.y + m_rowHeight - 2.0f
        };
        ctx.DrawText(node.IsExpanded() ? L"\x25BC" : L"\x25B6", arrowRect,
            selected ? Theme::SemanticColor::TextPrimary : Theme::SemanticColor::TextSecondary);
    }

    const float iconX = cx + ExpandArrowWidth;
    if (node.GetIcon() != 0)
    {
        const std::wstring glyph = CodepointToUtf16(node.GetIcon());
        const D2D1_RECT_F iconRect = {
            iconX, slot.y + 2.0f,
            iconX + IconWidth, slot.y + m_rowHeight - 2.0f
        };
        ctx.DrawText(glyph, iconRect, Theme::SemanticColor::TextSecondary);
    }

    const D2D1_RECT_F textRect = {
        iconX + (node.GetIcon() != 0 ? IconWidth : 0.0f) + 2.0f, slot.y + 2.0f,
        slot.x + slot.width - 4.0f, slot.y + slot.height - 2.0f
    };
    ctx.DrawText(node.GetText(), textRect,
        (selected || focused) ? Theme::SemanticColor::TextPrimary
                              : (hovered ? Theme::SemanticColor::TextPrimary
                                         : Theme::SemanticColor::TextPrimary));
}

void UITreeView::RenderScrollBar(RenderContext& ctx) noexcept
{
    const float contentHeight = GetContentHeight();
    const float maxScroll = (std::max)(0.0f, contentHeight - GetHeight());
    if (maxScroll <= 0.0f)
        return;

    const float viewportH = GetHeight();
    const float contentH = viewportH + maxScroll;
    const float thumbSize = (std::max)(20.0f, (viewportH / contentH) * viewportH);
    const float thumbPos = (m_scrollOffset / maxScroll) * (viewportH - thumbSize);

    const D2D1_RECT_F track = {
        GetX() + GetWidth() - ScrollBarWidth, GetY(),
        GetX() + GetWidth(), GetY() + viewportH
    };
    ctx.FillRectangle(track, Theme::SemanticColor::ControlFill, 0.5f);

    const D2D1_RECT_F thumb = {
        track.left + 1.0f, GetY() + thumbPos,
        track.right - 1.0f, GetY() + thumbPos + thumbSize
    };
    ctx.FillRoundedRect(thumb, Theme::SemanticColor::Accent, 3.0f, 3.0f, 0.85f);
}

// ── Mouse ─────────────────────────────────────────────────────────────

bool UITreeView::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::MouseMove && std::abs(args.wheelDelta) > 0.1f)
    {
        const float delta = args.wheelDelta > 0.0f ? -1.0f : 1.0f;
        m_scrollOffset += delta * m_rowHeight * 3.0f;
        InvalidateVisual();
        return true;
    }

    switch (type)
    {
    case EventType::MouseMove:
        m_hoveredIndex = -1;
        if (const auto* node = HitTestNodeAt(args.x, args.y))
        {
            for (int64_t i = 0; i < static_cast<int64_t>(m_visible.size()); ++i)
            {
                if (m_visible[static_cast<size_t>(i)] == node)
                {
                    m_hoveredIndex = i;
                    break;
                }
            }
        }
        InvalidateVisual();
        return true;

    case EventType::MouseDown:
        if (args.button == Input::MouseButton::Left)
        {
            auto* node = HitTestNodeAt(args.x, args.y);
            if (!node)
            {
                if (m_selection.GetSelectedCount() > 0)
                    m_selection.Clear();
                return true;
            }

            const int64_t index = GetVisibleIndex(*node);
            if (index < 0)
                return true;

            const int depth = node->GetDepth() - 1;
            const float arrowRight = GetX() + static_cast<float>(depth) * m_indentWidth + ExpandArrowWidth;

            if (!node->IsLeaf() && args.x < arrowRight)
            {
                ToggleNode(*node);
                return true;
            }

            m_selection.OnPointerClick(index, args.ctrl, args.shift);
            return true;
        }
        return false;

    case EventType::DoubleClick:
    {
        auto* node = HitTestNodeAt(args.x, args.y);
        if (!node)
            return false;

        if (!node->IsLeaf())
            ToggleNode(*node);

        if (m_onNodeActivated)
            m_onNodeActivated(*this, *node);
        return true;
    }

    default:
        return false;
    }
}

bool UITreeView::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type != EventType::KeyDown)
        return false;

    const int64_t total = static_cast<int64_t>(m_visible.size());
    if (total == 0)
        return false;

    auto current = m_selection.GetCurrent();
    if (current < 0)
        current = 0;

    auto move = [&](int64_t target) {
        target = (std::clamp)(target, int64_t{ 0 }, total - 1);
        m_selection.OnKeyMove(target, args.ctrl, args.shift);
        EnsureVisible(target);
        InvalidateVisual();
    };

    switch (args.key)
    {
    case Input::KeyCode::Down:
        move(current + 1);
        return true;

    case Input::KeyCode::Up:
        move(current - 1);
        return true;

    case Input::KeyCode::Right:
    {
        auto* node = GetVisibleNode(current);
        if (!node)
            return true;
        if (!node->IsExpanded())
            ExpandNode(*node);
        else if (!node->IsLeaf() && current + 1 < total)
            move(current + 1);
        return true;
    }

    case Input::KeyCode::Left:
    {
        auto* node = GetVisibleNode(current);
        if (!node)
            return true;
        if (node->IsExpanded())
            CollapseNode(*node);
        else if (node->GetParent() && node->GetParent() != m_root.get())
        {
            const int64_t parentIndex = GetVisibleIndex(*node->GetParent());
            if (parentIndex >= 0)
                move(parentIndex);
        }
        return true;
    }

    case Input::KeyCode::PageDown:
    {
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_rowHeight) - 1);
        move(current + page);
        return true;
    }

    case Input::KeyCode::PageUp:
    {
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_rowHeight) - 1);
        move(current - page);
        return true;
    }

    case Input::KeyCode::Home:
        move(0);
        return true;

    case Input::KeyCode::End:
        move(total - 1);
        return true;

    case Input::KeyCode::Return:
    {
        auto* node = GetVisibleNode(current);
        if (node && m_onNodeActivated)
        {
            m_onNodeActivated(*this, *node);
            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

bool UITreeView::OnFocusEvent(const FocusEventArgs&) noexcept
{
    InvalidateVisual();
    return false;
}

// ── Private helpers ───────────────────────────────────────────────────

void UITreeView::OnSelectionChanged(const SelectionManager& sm) noexcept
{
    if (m_onSelectionChanged)
        m_onSelectionChanged(sm);
    InvalidateVisual();
}

void UITreeView::EnsureVisible(int64_t index) noexcept
{
    const float top = static_cast<float>(index) * m_rowHeight;
    const float bottom = top + m_rowHeight;
    const float viewportH = GetHeight();

    float newScroll = m_scrollOffset;
    if (top < m_scrollOffset)
        newScroll = top;
    else if (bottom > m_scrollOffset + viewportH)
        newScroll = bottom - viewportH;

    m_scrollOffset = (std::max)(0.0f, newScroll);
    InvalidateVisual();
}

} // namespace DragonOS::DragonUI
