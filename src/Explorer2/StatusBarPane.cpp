#include <Explorer2/StatusBarPane.hpp>
#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Explorer2 {

StatusBarPane::StatusBarPane() noexcept
{
    m_statusBar = std::make_unique<UIDockPanel>();
    m_statusBar->SetAccessibleName(L"Explorer status bar");
    m_statusBar->SetAutomationId(L"Explorer2/StatusBar");
}

// ============================================================================
//  Item summary
// ============================================================================

void StatusBarPane::SetItemSummary(size_t totalEntries, size_t selectedCount,
                                   size_t filteredOut) noexcept
{
    wchar_t buf[128];
    if (filteredOut > 0)
    {
        swprintf_s(buf, L"%zu items | %zu selected | %zu filtered out",
                   totalEntries, selectedCount, filteredOut);
    }
    else
    {
        swprintf_s(buf, L"%zu items | %zu selected", totalEntries, selectedCount);
    }
    m_summary = buf;
    // Update the status bar display - the host window handles rendering
}

void StatusBarPane::SetOperation(const std::wstring& text, float progress01) noexcept
{
    // Update operation text - host window handles rendering
    // If there's a progress bar child, update it
    (void)progress01;
}

void StatusBarPane::ClearOperation() noexcept
{
    m_summary.clear();
}

void StatusBarPane::SetDiskFree(uint64_t freeBytes) noexcept
{
    if (freeBytes == 0)
    {
        m_freeText = L"Unknown";
    }
    else if (freeBytes >= 1024 * 1024 * 1024)
    {
        wchar_t buf[64];
        swprintf_s(buf, L"%llu GB free", (unsigned long long)(freeBytes / (1024LL * 1024LL * 1024LL)));
        m_freeText = buf;
    }
    else if (freeBytes >= 1024 * 1024)
    {
        wchar_t buf[64];
        swprintf_s(buf, L"%llu MB free", (unsigned long long)(freeBytes / (1024LL * 1024LL)));
        m_freeText = buf;
    }
    else
    {
        wchar_t buf[64];
        swprintf_s(buf, L"%llu KB free", (unsigned long long)freeBytes);
        m_freeText = buf;
    }
    // Host window handles rendering
}

// ============================================================================
//  Static helper
// ============================================================================

uint64_t StatusBarPane::QueryDiskFreeBytes(const std::wstring& rootPath) noexcept
{
    // Host window / SDK handles this via FileSystemService
    return 0;
}

// ============================================================================
//  (No SetAutomationPeer - AccessibilityManager creates peers externally)
// ============================================================================

} // namespace DragonOS::Explorer2