#include <DragonUI/Controls/ContextMenu.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

uint64_t UIContextMenu::AddItem(std::wstring_view text, ItemCallback cb, uint32_t icon) noexcept
{
    uint64_t id = NextId();
    ContextItem ctxItem;
    ctxItem.id = id;
    ctxItem.text = text;
    ctxItem.icon = icon;
    ctxItem.callback = std::move(cb);
    m_items.push_back(std::move(ctxItem));
    RebuildMenu();
    return id;
}

void UIContextMenu::AddSeparator() noexcept
{
    ContextItem ctxItem;
    ctxItem.id = NextId();
    ctxItem.isSeparator = true;
    m_items.push_back(std::move(ctxItem));
    RebuildMenu();
}

uint64_t UIContextMenu::AddSubMenu(std::wstring_view text, std::unique_ptr<UIMenu> submenu, uint32_t icon) noexcept
{
    uint64_t id = NextId();
    ContextItem ctxItem;
    ctxItem.id = id;
    ctxItem.text = text;
    ctxItem.icon = icon;
    ctxItem.submenu = std::move(submenu);
    m_items.push_back(std::move(ctxItem));
    RebuildMenu();
    return id;
}

void UIContextMenu::ClearItems() noexcept
{
    m_items.clear();
    m_itemIdMap.clear();
    if (m_menu) m_menu->ClearItems();
}

void UIContextMenu::ShowAt(float x, float y) noexcept
{
    m_showX = x;
    m_showY = y;
    m_isOpen = true;
    RebuildMenu();
    if (m_menu)
    {
        auto slot = m_menu->MeasureOverride({0, 0, 400, 800});
        m_menu->SetPopupPosition(x, y);
        m_menu->SetBounds({x, y, slot.width, slot.height});
        m_menu->SetOpen(true);
    }
}

void UIContextMenu::Close() noexcept
{
    m_isOpen = false;
    if (m_menu)
    {
        m_menu->SetOpen(false);
        m_menu->CloseSubmenus();
    }
}

UIMenuItem* UIContextMenu::FindItemById(uint64_t id) noexcept
{
    for (auto& [item, itemId] : m_itemIdMap)
    {
        if (itemId == id)
            return item;
    }
    return nullptr;
}

void UIContextMenu::RebuildMenu() noexcept
{
    if (!m_menu)
    {
        m_menu = std::make_unique<UIMenu>();
    }

    m_menu->ClearItems();
    m_itemIdMap.clear();

    for (auto& ctxItem : m_items)
    {
        if (ctxItem.isSeparator)
        {
            m_menu->AddSeparator();
        }
        else if (ctxItem.submenu)
        {
            auto* added = m_menu->AddSubMenuItem(ctxItem.text, std::move(ctxItem.submenu));
            if (added)
            {
                added->SetIcon(ctxItem.icon);
                m_itemIdMap[added] = ctxItem.id;
            }
        }
        else
        {
            auto* added = m_menu->AddItem(ctxItem.text,
                [this, id = ctxItem.id]() {
                    for (auto& ci : m_items)
                    {
                        if (ci.id == id && ci.callback)
                        {
                            ci.callback();
                            Close();
                            return;
                        }
                    }
                });
            if (added)
            {
                added->SetIcon(ctxItem.icon);
                added->SetChecked(ctxItem.checked);
                added->SetShortcut(ctxItem.shortcut);
                added->SetEnabled(ctxItem.enabled);
                m_itemIdMap[added] = ctxItem.id;
            }
        }
    }
}

void UIContextMenu::Render(RenderContext& ctx) noexcept
{
    if (!m_isOpen || !m_menu) return;
    m_menu->Render(ctx);
}

DesiredSize UIContextMenu::MeasureOverride(const LayoutSlot& available) noexcept
{
    if (m_menu)
        return m_menu->MeasureOverride(available);
    return {0, 0};
}

void UIContextMenu::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    if (m_menu)
        m_menu->Arrange(finalSlot);
}

bool UIContextMenu::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (!m_isOpen || !m_menu) return false;
    return m_menu->OnMouseEvent(type, args);
}

bool UIContextMenu::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (!m_isOpen || !m_menu) return false;
    return m_menu->OnKeyEvent(type, args);
}

} // namespace
