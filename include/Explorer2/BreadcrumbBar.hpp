// ============================================================================
//  BreadcrumbBar.hpp — Address bar with breadcrumb segments (Part 4).
//
//  Displays the current path as clickable segment buttons with an overflow
//  ellipsis, a history drop-down, and an inline edit mode (text box + Go)
//  toggled by the window (F6 / click empty space / Ctrl+L style flows).
//
//  All DragonUI controls are used exclusively.  Accessibility is handled by
//  the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/ContextMenu.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/TextBox.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

class BreadcrumbBar final {
public:
    using NavigateFn = std::function<void(const std::wstring&)>;

    BreadcrumbBar() noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_root.get(); }
    /// Transfers the root element into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_root);
    }

    void SetOnNavigate(NavigateFn cb) noexcept { m_onNavigate = std::move(cb); }

    /// Hands a built menu to the host window's overlay system for display.
    using MenuSink = std::function<void(std::unique_ptr<DragonUI::UIContextMenu>, float, float)>;
    void SetMenuSink(MenuSink sink) noexcept { m_menuSink = std::move(sink); }

    /// Supplies recent locations for the history drop-down.
    void SetHistoryProvider(std::function<std::vector<std::wstring>()> provider) noexcept
    {
        m_historyProvider = std::move(provider);
    }

    /// Rebuilds segments for @p path.
    void SetPath(const std::wstring& path) noexcept;

    void SetEditing(bool editing) noexcept { m_editing = editing; }
    [[nodiscard]] bool IsEditing() const noexcept { return m_editing; }
    void BeginEditing() noexcept { SetEditing(true); }
    void CancelEditing() noexcept { SetEditing(false); }

    /// Focuses the edit text box (window routes this through its FocusManager).
    [[nodiscard]] DragonUI::Element* GetEditBox() noexcept { return m_editBox.get(); }

private:
    void BuildSegments(const std::wstring& path) noexcept;
    void ShowHistoryMenu(float x, float y) noexcept;
    void ShowOverflowMenu(float x, float y, std::vector<std::wstring> hiddenPaths) noexcept;

    NavigateFn m_onNavigate;
    MenuSink m_menuSink;
    std::function<std::vector<std::wstring>()> m_historyProvider;

    std::unique_ptr<DragonUI::Element> m_root{std::make_unique<DragonUI::UIDockPanel>()};
    std::unique_ptr<DragonUI::Element> m_editBox;      // Inline edit text box
    std::unique_ptr<DragonUI::Element> m_editRow;      // Edit mode row

    bool m_editing{};

    // Accessibility peers are created by the host's AccessibilityManager
};
} // namespace DragonOS::Explorer2