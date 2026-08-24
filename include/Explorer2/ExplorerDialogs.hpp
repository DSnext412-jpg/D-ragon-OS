// ============================================================================
//  ExplorerDialogs.hpp — Modal dialog factories built on DragonUI UIDialog.
// ============================================================================

#pragma once

#include <FileSystem/FileEntry.hpp>
#include <DragonUI/Dialogs/UIDialog.hpp>

#include <functional>
#include <memory>
#include <string>

namespace DragonOS::Explorer2 {

class ExplorerDialogs final {
public:
    using TextCallback = std::function<void(const std::wstring&)>;

    /// "New folder" — commits @p callback with the chosen name.
    [[nodiscard]] static std::unique_ptr<DragonUI::UIDialog> CreateNewFolder(
        const TextCallback& onCommit) noexcept;

    /// Rename dialog prefilled with @p currentName.
    [[nodiscard]] static std::unique_ptr<DragonUI::UIDialog> CreateRename(
        const std::wstring& currentName, const TextCallback& onCommit) noexcept;

    /// Read-only properties sheet for a single entry.
    [[nodiscard]] static std::unique_ptr<DragonUI::UIDialog> CreateProperties(
        const FileSystem::FileEntry& entry) noexcept;

    /// Positions @p dialog centered over the window client rect.
    static void CenterOverClient(
        DragonUI::UIDialog& dialog,
        float clientX, float clientY, float clientW, float clientH,
        float viewportW, float viewportH) noexcept;

private:
    ExplorerDialogs() = delete;
};

} // namespace DragonOS::Explorer2
