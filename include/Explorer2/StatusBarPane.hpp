// ============================================================================
//  StatusBarPane.hpp — Status bar (Part 9).
//
//  Left panel : background-operation text + progress bar (loads, file ops).
//  Right panel: item counts, selection count and free disk space (TTL-cached
//               per drive root via GetDiskFreeSpaceExW).
//
//  Accessibility is handled by the host's AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Core/Element.hpp>

#include <string>

namespace DragonOS::Explorer2 {

class StatusBarPane final {
public:
    StatusBarPane() noexcept;

    /// "N items | M selected | K filtered out" summary for the active tab.
    void SetItemSummary(size_t totalEntries, size_t selectedCount,
                        size_t filteredOut) noexcept;

    /// Background operation line; pass progress in [0,1] or -1 for none.
    void SetOperation(const std::wstring& text, float progress01 = -1.0f) noexcept;
    void ClearOperation() noexcept;

    void SetDiskFree(uint64_t freeBytes) noexcept;

    [[nodiscard]] DragonUI::Element* GetControl() noexcept { return m_statusBar.get(); }
    /// Transfers the status bar into the owning UI tree (assembly-time).
    [[nodiscard]] std::unique_ptr<DragonUI::Element> TakeRoot() noexcept
    {
        return std::move(m_statusBar);
    }

    /// Queries the OS for the free space of @p rootPath's drive (cached 2 s).
    static uint64_t QueryDiskFreeBytes(const std::wstring& rootPath) noexcept;

private:
    void RefreshSummaryText() noexcept;

    std::unique_ptr<DragonUI::Element> m_statusBar{};
    std::wstring m_summary;
    std::wstring m_freeText;

    // Accessibility is handled by the host's AccessibilityManager
};
} // namespace DragonOS::Explorer2