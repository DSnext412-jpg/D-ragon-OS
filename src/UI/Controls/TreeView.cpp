#include <UI/Controls/TreeView.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

TreeView::TreeView() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultListView());
    SetAccessibilityRole(L"tree");
}

int TreeView::AddItem(int parentIndex, std::wstring_view text) noexcept
{
    auto item = std::make_unique<TreeViewItem>();
    item->text = text;

    if (parentIndex < 0)
    {
        item->depth = 0;
        m_rootItems.push_back(std::move(item));
        int idx = static_cast<int>(m_rootItems.size()) - 1;
        FlatternTree();
        return idx;
    }

    int current = 0;
    TreeViewItem* parent = FindItem(parentIndex, m_rootItems, current);
    if (!parent) return -1;

    item->depth = parent->depth + 1;
    item->parent = parent;
    parent->children.push_back(std::move(item));
    FlatternTree();
    return static_cast<int>(m_flatItems.size()) - 1;
}

bool TreeView::RemoveItem(int index) noexcept
{
    if (index < 0) return false;

    TreeViewItem* item = GetItem(index);
    if (!item) return false;

    int current = 0;
    if (item->parent == nullptr)
    {
        for (auto it = m_rootItems.begin(); it != m_rootItems.end(); ++it)
        {
            if (it->get() == item)
            {
                m_rootItems.erase(it);
                FlatternTree();
                return true;
            }
            ++current;
        }
    }
    else
    {
        for (auto it = item->parent->children.begin(); it != item->parent->children.end(); ++it)
        {
            if (it->get() == item)
            {
                item->parent->children.erase(it);
                FlatternTree();
                return true;
            }
        }
    }
    return false;
}

void TreeView::ClearItems() noexcept
{
    m_rootItems.clear();
    m_flatItems.clear();
    m_selectedIndex = -1;
    InvalidateVisual();
}

void TreeView::Expand(int index) noexcept
{
    TreeViewItem* item = GetItem(index);
    if (item && !item->children.empty() && !item->isExpanded)
    {
        item->isExpanded = true;
        FlatternTree();
        InvalidateLayout();
    }
}

void TreeView::Collapse(int index) noexcept
{
    TreeViewItem* item = GetItem(index);
    if (item && item->isExpanded)
    {
        item->isExpanded = false;
        FlatternTree();
        InvalidateLayout();
    }
}

bool TreeView::IsExpanded(int index) const noexcept
{
    TreeViewItem* item = GetItem(index);
    return item && item->isExpanded;
}

void TreeView::SetSelectedIndex(int index) noexcept
{
    if (index < -1 || index >= static_cast<int>(m_flatItems.size()))
        return;
    if (m_selectedIndex != index)
    {
        m_selectedIndex = index;
        if (m_onSelectionChanged)
            m_onSelectionChanged(m_selectedIndex);
        InvalidateVisual();
    }
}

TreeViewItem* TreeView::GetItem(int index) const noexcept
{
    if (index >= 0 && index < static_cast<int>(m_flatItems.size()))
        return m_flatItems[index];
    return nullptr;
}

void TreeView::FlatternTree() noexcept
{
    m_flatItems.clear();
    for (const auto& root : m_rootItems)
    {
        m_flatItems.push_back(root.get());
        if (root->isExpanded)
        {
            FlattenChildren(root.get());
        }
    }
}

void TreeView::FlattenChildren(TreeViewItem* parent) noexcept
{
    for (const auto& child : parent->children)
    {
        m_flatItems.push_back(child.get());
        if (child->isExpanded)
        {
            FlattenChildren(child.get());
        }
    }
}

TreeViewItem* TreeView::FindItem(int index, std::vector<std::unique_ptr<TreeViewItem>>& items, int& current) noexcept
{
    for (const auto& item : items)
    {
        if (current == index) return item.get();
        ++current;
    }
    return nullptr;
}

void TreeView::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    renderer.PushClip(bounds);

    for (int i = 0; i < static_cast<int>(m_flatItems.size()); ++i)
    {
        auto* item = m_flatItems[i];
        if (!item) continue;

        float y = bounds.top + i * ItemHeight;
        if (y + ItemHeight < bounds.top || y > bounds.bottom) continue;

        D2D1_RECT_F itemRect = {bounds.left, y, bounds.right, y + ItemHeight};

        if (i == m_selectedIndex)
        {
            StyleStateColors selColors;
            selColors.background = Theme::SemanticColor::Selection;
            renderer.FillBackground(itemRect, selColors, 0.0f, opacity);
        }
        else if (i == m_hoveredIndex)
        {
            StyleStateColors hoverColors;
            hoverColors.background = Theme::SemanticColor::Hover;
            renderer.FillBackground(itemRect, hoverColors, 0.0f, opacity);
        }

        float indent = static_cast<float>(item->depth) * IndentWidth;
        bool hasChildren = !item->children.empty();

        if (hasChildren)
        {
            D2D1_RECT_F arrowRect = {
                bounds.left + indent + 2.0f, y + 4.0f,
                bounds.left + indent + 20.0f, y + ItemHeight - 4.0f
            };
            const auto& arrowColors = style->ResolveState(ElementState::Normal);
            renderer.DrawIcon(arrowRect, item->isExpanded ? L'\u25BC' : L'\u25B6', arrowColors, 10.0f);
        }

        float textIndent = hasChildren ? indent + 22.0f : indent + 6.0f;
        D2D1_RECT_F textRect = {
            bounds.left + textIndent, y + 2.0f,
            bounds.right - 6.0f, y + ItemHeight - 2.0f
        };
        const auto& textColors = style->ResolveState(i == m_selectedIndex ? ElementState::Focused : ElementState::Normal);
        renderer.DrawText(item->text, textRect, textColors);
    }

    if (GetState() == ElementState::Focused && m_selectedIndex >= 0)
    {
        float selY = bounds.top + m_selectedIndex * ItemHeight;
        D2D1_RECT_F focusRect = {bounds.left, selY, bounds.right, selY + ItemHeight};
        renderer.DrawFocusIndicator(focusRect);
    }

    renderer.PopClip();
}

bool TreeView::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        SetState(ElementState::Normal);
        m_hoveredIndex = -1;
        InvalidateVisual();
        return true;

    case UIEventType::MouseMove:
    {
        if (event.x >= bounds.left && event.x <= bounds.right &&
            event.y >= bounds.top && event.y <= bounds.bottom)
        {
            int idx = static_cast<int>((event.y - bounds.top) / ItemHeight);
            if (idx >= 0 && idx < static_cast<int>(m_flatItems.size()))
            {
                if (m_hoveredIndex != idx)
                {
                    m_hoveredIndex = idx;
                    InvalidateVisual();
                }
                return true;
            }
        }
        if (m_hoveredIndex != -1)
        {
            m_hoveredIndex = -1;
            InvalidateVisual();
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        int idx = static_cast<int>((event.y - bounds.top) / ItemHeight);
        if (idx < 0 || idx >= static_cast<int>(m_flatItems.size())) return false;

        auto* item = m_flatItems[idx];
        if (!item) return false;

        float indent = static_cast<float>(item->depth) * IndentWidth;
        bool hasChildren = !item->children.empty();
        float arrowRight = hasChildren ? (bounds.left + indent + 20.0f) : (bounds.left + indent + 6.0f);

        if (hasChildren && event.x <= arrowRight)
        {
            if (item->isExpanded) Collapse(idx); else Expand(idx);
            return true;
        }

        SetSelectedIndex(idx);
        return true;
    }

    case UIEventType::KeyDown:
        switch (event.key)
        {
        case Input::KeyCode::Down:
            SetSelectedIndex((std::min)(m_selectedIndex + 1, static_cast<int>(m_flatItems.size()) - 1));
            return true;
        case Input::KeyCode::Up:
            SetSelectedIndex((std::max)(m_selectedIndex - 1, 0));
            return true;
        case Input::KeyCode::Right:
            if (m_selectedIndex >= 0) Expand(m_selectedIndex);
            return true;
        case Input::KeyCode::Left:
            if (m_selectedIndex >= 0) Collapse(m_selectedIndex);
            return true;
        case Input::KeyCode::Return:
            if (m_selectedIndex >= 0)
            {
                auto* item = m_flatItems[m_selectedIndex];
                if (item && !item->children.empty())
                {
                    if (item->isExpanded) Collapse(m_selectedIndex); else Expand(m_selectedIndex);
                }
            }
            return true;
        default:
            return false;
        }

    case UIEventType::FocusGained:
        SetState(ElementState::Focused);
        return true;

    case UIEventType::FocusLost:
        SetState(ElementState::Normal);
        return true;

    default:
        return false;
    }
}

} // namespace
