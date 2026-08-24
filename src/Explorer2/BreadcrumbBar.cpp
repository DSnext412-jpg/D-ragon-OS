#include <Explorer2/BreadcrumbBar.hpp>
#include <Explorer2/ExplorerTypes.hpp>

#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/Controls/ContextMenu.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/TextBox.hpp>

#include <functional>
#include <memory>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction
// ============================================================================

BreadcrumbBar::BreadcrumbBar() noexcept
{
    m_root->SetAccessibleName(L"Breadcrumb address bar");
    m_root->SetAutomationId(L"Explorer2/Breadcrumb");
}

// ============================================================================
//  Path management
// ============================================================================

void BreadcrumbBar::SetPath(const std::wstring& path) noexcept
{
    m_root->SetVisible(!path.empty());

    if (path.empty())
    {
        // Root drive — show single "This PC" segment
        auto* rootBtn = m_root->AddChild(std::make_unique<UIButton>(L"::"));
        rootBtn->SetAccessibleName(L"Root drive");
        rootBtn->SetAutomationId(L"Explorer2/Breadcrumb/Root");
        rootBtn->SetOnClick([this]() { if (m_onNavigate) m_onNavigate(L""); });
        return;
    }

    BuildSegments(path);
}

void BreadcrumbBar::BuildSegments(const std::wstring& path) noexcept
{
    // Split path into segments
    std::vector<std::wstring> segments;
    size_t start = 0;
    size_t end = path.find(L'\\', start);
    while (end != std::wstring::npos)
    {
        segments.push_back(path.substr(start, end - start));
        start = end + 1;
        end = path.find(L'\\', start);
    }
    segments.push_back(path.substr(start));

    // If more than 5 segments, show overflow ellipsis
    static constexpr size_t kMaxVisibleSegments = 5;
    bool hasOverflow = segments.size() > kMaxVisibleSegments;

    // Clear previous segments
    m_root->RemoveAllChildren();

    for (size_t i = 0; i < segments.size(); ++i)
    {
        if (i >= kMaxVisibleSegments - 1)
        {
            // Show overflow ellipsis instead of remaining segments
            if (hasOverflow)
            {
                auto* ellipsisBtn = m_root->AddChild(std::make_unique<UIButton>(L"..."));
                ellipsisBtn->SetAccessibleName(L"Overflow ellipsis");
                ellipsisBtn->SetAutomationId(L"Explorer2/Breadcrumb/Overflow");
                ellipsisBtn->SetOnClick([this]() { ShowOverflowMenu(0.0f, 0.0f, {}); });
            }
            break;
        }

        const std::wstring& seg = segments[i];
        auto* segBtn = m_root->AddChild(std::make_unique<UIButton>(seg));
        segBtn->SetAccessibleName(L"Breadcrumb segment");
        segBtn->SetAutomationId(L"Explorer2/Breadcrumb/Segment/" + seg);
        segBtn->SetMinSize(0.0f, 28.0f);
        segBtn->SetOnClick([this, segPath = seg](UIButton&) {
            if (m_onNavigate)
            {
                m_onNavigate(segPath);
            }
        });
    }

    // Append history drop-down button at the right end
    auto* historyBtn = m_root->AddChild(std::make_unique<UIButton>(L"▾"));
    historyBtn->SetAccessibleName(L"History drop-down");
    historyBtn->SetAutomationId(L"Explorer2/Breadcrumb/HistoryBtn");
    historyBtn->SetMinSize(24.0f, 24.0f);
    historyBtn->SetOnClick([this](UIButton&) { ShowHistoryMenu(0.0f, 0.0f); });
}

void BreadcrumbBar::ShowHistoryMenu(float x, float y) noexcept
{
    if (!m_historyProvider) { return; }

    auto menu = std::make_unique<UIContextMenu>();

    auto items = m_historyProvider();

    for (const auto& item : items)
    {
        UIMenuItem* mi = menu->AddItem(item.c_str(),
            [this, itemPath = item]() {
                if (m_onNavigate) m_onNavigate(itemPath);
            });
        if (mi)
        {
            // Can't set check state easily without current path reference
        }
    }

    // Route to the window's menu sink for overlay display
    if (m_menuSink)
    {
        m_menuSink(std::move(menu), x, y);
    }
}

void BreadcrumbBar::ShowOverflowMenu(float x, float y, std::vector<std::wstring> hiddenPaths) noexcept
{
    if (!m_historyProvider) { return; }

    auto menu = std::make_unique<UIContextMenu>();
    auto items = m_historyProvider();

    for (const auto& item : items)
    {
        UIMenuItem* mi = menu->AddItem(item.c_str(),
            [this, itemPath = item]() {
                if (m_onNavigate) m_onNavigate(itemPath);
            });
    }

    if (m_menuSink)
    {
        m_menuSink(std::move(menu), x, y);
    }
}

// ============================================================================
//  Editing
// ============================================================================

void BreadcrumbBar::BeginEditing() noexcept
{
    m_editing = true;
    // Host will rearrange visibility
}

void BreadcrumbBar::CancelEditing() noexcept
{
    m_editing = false;
    // Host will rearrange visibility
}

// Accessor for current path - implemented by window
std::wstring BreadcrumbBar::GetCurrentPath() const noexcept
{
    return {};
}

// ============================================================================
//  (No SetAutomationPeer - AccessibilityManager creates peers externally)
// ============================================================================

} // namespace DragonOS::Explorer2