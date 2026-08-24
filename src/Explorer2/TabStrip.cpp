#include <Explorer2/TabStrip.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction
// ============================================================================

TabStrip::TabStrip() noexcept
{
    SetAccessibleName(L"Folder tabs");
    SetAutomationId(L"Explorer2/Tabs");
}

// ============================================================================
//  Host / model wiring
// ============================================================================

void TabStrip::SetHost(Host host) noexcept
{
    m_host = std::move(host);
    Rebuild();
}

// ============================================================================
//  Cell building
// ============================================================================

void TabStrip::Rebuild() noexcept
{
    ClearChildren();
    m_cells.clear();

    const std::vector<TabState>& tabs = m_host.getTabs ? m_host.getTabs() : std::vector<TabState>{};
    const uint64_t activeId = m_host.getActiveTabId ? m_host.getActiveTabId() : 0;

    float x = 0.0f;
    for (const TabState& tab : tabs)
    {
        auto cellPanel = std::make_unique<UIDockPanel>();
        Element* panelRaw = cellPanel.get();

        auto titleButton = std::make_unique<Button>(tab.title);
        Button* title = titleButton.get();
        title->SetAccessibleName(L"Tab " + tab.title);
        title->SetAutomationId(L"Explorer2/Tabs/Tab/" + tab.title);
        title->SetOnClick([this, id = tab.id](Button&) {
            if (m_host.onActivate)
            {
                m_host.onActivate(id);
            }
        });

        auto closeButton = std::make_unique<Button>(L"\x2715"); // ✕
        Button* close = closeButton.get();
        close->SetAccessibleName(L"Close tab " + tab.title);
        close->SetAutomationId(L"Explorer2/Tabs/Close/" + tab.title);
        close->SetMinSize(kCloseButtonWidth, kCloseButtonWidth);
        close->SetOnClick([this, id = tab.id](Button&) {
            if (m_host.onClose)
            {
                m_host.onClose(id);
            }
        });

        panelRaw->AddChild(std::move(closeButton));
        panelRaw->SetChildDock(*close, Dock::Right);
        panelRaw->AddChild(std::move(titleButton));
        panelRaw->SetChildDock(*title, Dock::Fill);
        panelRaw->SetLastChildFill(true);

        AddChild(std::move(cellPanel));

        Cell cell;
        cell.tabId = tab.id;
        cell.x = x;
        cell.width = kCellPadding + EstimateCellWidth(tab.title) + kCloseButtonWidth + kCellPadding;
        cell.panel = panelRaw;
        cell.titleButton = title;
        cell.closeButton = close;
        m_cells.push_back(cell);

        x += cell.width + 2.0f;

        // Active-tab affordance via control state.
        if (title && tab.id == activeId)
        {
            title->SetControlState(ControlState::Pressed);
        }
    }

    // New tab "+" button
    if (m_host.onNewTabRequest)
    {
        auto newTab = std::make_unique<Button>(L"+");
        Button* plus = newTab.get();
        plus->SetAccessibleName(L"New tab");
        plus->SetAutomationId(L"Explorer2/Tabs/New");
        plus->SetMinSize(kNewTabWidth, kStripHeight - 6.0f);
        plus->SetOnClick([this](Button&) { m_host.onNewTabRequest(); });
        AddChild(std::move(newTab));
    }

    InvalidateLayout();
}

float TabStrip::EstimateCellWidth(const std::wstring& title) noexcept
{
    return static_cast<float>(title.size()) * 7.0f + 24.0f;
}

// ============================================================================
//  Hit testing
// ============================================================================

int TabStrip::HitTestCell(float x, float y) const noexcept
{
    if (y < GetY() || y >= GetY() + GetHeight())
    {
        return -1;
    }
    for (size_t i = 0; i < m_cells.size(); ++i)
    {
        if (x >= m_cells[i].x && x < m_cells[i].x + m_cells[i].width)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

size_t TabStrip::InsertionIndexForX(float x) const noexcept
{
    size_t index = 0;
    for (const Cell& cell : m_cells)
    {
        if (x > cell.x + cell.width * 0.5f)
        {
            ++index;
        }
    }
    return index;
}

// ============================================================================
//  Input handling
// ============================================================================

bool TabStrip::HandleMousePress(float x, float y, bool middleButton) noexcept
{
    const int index = HitTestCell(x, y);
    if (index < 0)
    {
        return false;
    }

    const uint64_t id = m_cells[static_cast<size_t>(index)].tabId;

    if (middleButton)
    {
        if (m_host.onClose)
        {
            m_host.onClose(id);
        }
        return true;
    }

    m_pressedIndex = index;
    m_pressX = x;
    m_dragMoved = false;

    // Activate immediately (browser-style).
    if (m_host.onActivate)
    {
        m_host.onActivate(id);
    }
    return true;
}

bool TabStrip::HandleMouseMove(float x, float y, bool leftHeld) noexcept
{
    if (!leftHeld || m_pressedIndex < 0)
    {
        return false;
    }

    if (!m_dragMoved && std::fabs(x - m_pressX) > kDragThreshold)
    {
        m_dragIndex = m_pressedIndex;
        m_dragMoved = true;
    }

    if (m_dragIndex >= 0)
    {
        const size_t target = InsertionIndexForX(x);
        const size_t source = static_cast<size_t>(m_dragIndex);
        if (target != source && m_host.onReorder)
        {
            m_host.onReorder(source, target);
            m_dragIndex = static_cast<int>(target);
            m_pressedIndex = m_dragIndex;
        }
        return true;
    }
    return false;
}

bool TabStrip::HandleMouseRelease(float x, float y) noexcept
{
    (void)x;
    (void)y;
    m_pressedIndex = -1;
    m_dragIndex = -1;
    m_dragMoved = false;
    return false;
}

// ============================================================================
//  (No SetAutomationPeer - AccessibilityManager creates peers externally)
// ============================================================================

} // namespace DragonOS::Explorer2