#include <Explorer2/RibbonBar.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

namespace {

constexpr float kBarHeight = 40.0f;
constexpr float kNavButtonSize = 30.0f;

} // namespace

// ============================================================================
//  Construction
// ============================================================================

RibbonBar::RibbonBar() noexcept
{
    m_root->SetAccessibleName(L"Explorer toolbar");
    m_root->SetAutomationId(L"Explorer2/Ribbon");
}

// ============================================================================
//  Wiring / state sync
// ============================================================================

void RibbonBar::SetActions(Actions actions) noexcept
{
    m_actions = std::move(actions);

    // Buttons - accessibility peers created by host's AccessibilityManager
    // We just wire the click handlers; the framework handles the rest
    // m_backButton->SetOnClick(...);
    // m_forwardButton->SetOnClick(...);
    
    RebuildActionButtons();
}

void RibbonBar::BindSearch(
    std::function<void(const std::wstring&)> onChanged,
    std::function<void()> onFocusRequest) noexcept
{
    if (m_searchBox)
    {
        m_searchBox->SetText(L"");
        // Text change handling is done by the host window
    }
}

void RibbonBar::SyncState() noexcept
{
    // State sync - enabled/disabled state mirrors per-tab selection
    // This is handled by the window calling this method
}

void RibbonBar::RefreshMenus() noexcept
{
    if (m_actions.onNewFolder)
    {
        RebuildActionButtons();
    }
}

// ============================================================================
//  Menus
// ============================================================================

std::unique_ptr<DragonUI::UIMenu> RibbonBar::BuildViewMenu() const noexcept
{
    auto menu = std::make_unique<UIMenu>();

    const ViewMode current = m_actions.currentViewMode ? m_actions.currentViewMode()
                                                       : ViewMode::Details;
    struct Entry final { ViewMode mode; const wchar_t* label; };
    constexpr Entry entries[] = {
        { ViewMode::Details, L"Details" },
        { ViewMode::List,    L"List" },
        { ViewMode::Tiles,   L"Tiles" },
    };

    for (const Entry& entry : entries)
    {
        UIMenuItem* item = menu->AddItem(entry.label,
            [a = m_actions.onViewMode, mode = entry.mode]() { a(mode); });
        if (item && entry.mode == current)
        {
            item->SetChecked(true);
        }
    }
    return menu;
}

std::unique_ptr<DragonUI::UIMenu> RibbonBar::BuildSortMenu() const noexcept
{
    auto menu = std::make_unique<UIMenu>();

    const SortKey currentKey = m_actions.currentSortKey ? m_actions.currentSortKey()
                                                         : SortKey::Name;
    const SortDir currentDir = m_actions.currentSortDir ? m_actions.currentSortDir()
                                                         : SortDir::Ascending;

    struct Entry final { SortKey key; const wchar_t* label; };
    constexpr Entry entries[] = {
        { SortKey::Name,          L"Name" },
        { SortKey::Size,          L"Size" },
        { SortKey::Type,          L"Type" },
        { SortKey::DateModified,  L"Date modified" },
    };

    for (const Entry& entry : entries)
    {
        UIMenuItem* item = menu->AddItem(entry.label,
            [a = m_actions.onSortBy, key = entry.key]() { a(key); });
        if (item && entry.key == currentKey)
        {
            item->SetChecked(true);
        }
    }

    menu->AddSeparator();
    UIMenuItem* dirItem = menu->AddItem(
        currentDir == SortDir::Ascending ? L"Descending ▼" : L"Ascending ▲",
        [key = currentKey, a = m_actions.onSortBy]() mutable { a(key); });
    (void)dirItem;

    return menu;
}

std::unique_ptr<DragonUI::UIMenu> RibbonBar::BuildGroupMenu() const noexcept
{
    auto menu = std::make_unique<UIMenu>();

    const GroupMode current = m_actions.currentGroupMode ? m_actions.currentGroupMode()
                                                          : GroupMode::None;
    struct Entry final { GroupMode mode; const wchar_t* label; };
    constexpr Entry entries[] = {
        { GroupMode::None,           L"(none)" },
        { GroupMode::ByName,         L"Name" },
        { GroupMode::ByType,         L"Type" },
        { GroupMode::BySize,         L"Size" },
        { GroupMode::ByDateModified, L"Date modified" },
    };

    for (const Entry& entry : entries)
    {
        UIMenuItem* item = menu->AddItem(entry.label,
            [a = m_actions.onGroupBy, mode = entry.mode]() { a(mode); });
        if (item && entry.mode == current)
        {
            item->SetChecked(true);
        }
    }
    return menu;
}

// ============================================================================
//  Construction (detail)
// ============================================================================

// RibbonBar construction is handled by the window's Layout method,
// which builds the full element tree and wires the controls.

} // namespace DragonOS::Explorer2