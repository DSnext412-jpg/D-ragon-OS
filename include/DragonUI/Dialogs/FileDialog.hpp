#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/Grid.hpp>
#include <DragonUI/Core/VirtualItemSource.hpp>

#include <FileSystem/FileSystemService.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  A single file dialog filter, e.g. { L"Text Files", { L"*.txt" } }.
 */
struct FileDialogFilter {
    std::wstring display;
    std::vector<std::wstring> patterns;
};

/**
 * @brief  Virtual item source over a snapshot of directory entries.
 */
class FileDialogSource final : public VirtualItemSource {
public:
    void SetEntries(const std::vector<FileSystem::FileEntry>* entries) noexcept
    {
        m_entries = entries;
    }

    [[nodiscard]] int64_t GetCount() const noexcept override
    {
        return m_entries ? static_cast<int64_t>(m_entries->size()) : 0;
    }

    std::any GetItem(int64_t index) const override
    {
        return (*m_entries)[static_cast<size_t>(index)];
    }

private:
    const std::vector<FileSystem::FileEntry>* m_entries{};
};

enum class FileDialogMode : uint8_t {
    Open,
    Save,
    Folder,
};

/**
 * @brief  Shared file-browser chrome for open/save/folder dialogs.
 *
 * Layout: a toolbar row (Up / Refresh / Home + current path), a UIListView
 * of entries (folders first), and a bottom row (filter + file-name box for
 * Open/Save, or the selected-folder label for Folder mode).
 */
class FileDialogBase : public UIDialog {
public:
    [[nodiscard]] const std::wstring& GetCurrentDirectory() const noexcept { return m_currentDir; }

protected:
    FileDialogBase(FileSystem::FileSystemService& fs,
                   std::wstring_view title,
                   std::wstring_view initialDir,
                   FileDialogMode mode,
                   std::wstring_view filters,
                   bool modal) noexcept;

    void BuildBrowser() noexcept;
    void NavigateTo(const std::wstring& path) noexcept;
    void RefreshList() noexcept;
    void ApplyFilter() noexcept;
    void OnItemActivated(int64_t displayIndex) noexcept;
    void OnSelectionChanged() noexcept;
    void CycleFilter() noexcept;
    void Commit() noexcept;

    [[nodiscard]] FileSystem::FileEntry EntryAt(int64_t displayIndex) const noexcept;
    [[nodiscard]] std::wstring SelectedPathFromEntry(int64_t displayIndex) const noexcept;
    [[nodiscard]] std::wstring NormalizeFilterPattern(std::wstring_view pattern) const noexcept;
    [[nodiscard]] bool MatchesActiveFilter(const FileSystem::FileEntry& entry) const noexcept;

    static std::vector<FileDialogFilter> ParseFilters(std::wstring_view spec) noexcept;
    static bool WildcardMatch(std::wstring_view pattern, std::wstring_view name) noexcept;

    FileSystem::FileSystemService& m_fs;
    FileDialogMode m_mode;
    std::wstring m_currentDir;
    std::vector<FileSystem::FileEntry> m_entries;
    std::shared_ptr<FileDialogSource> m_source;
    std::vector<FileDialogFilter> m_filters;
    size_t m_filterIndex{};

    UIListView* m_list{};
    UITextBox* m_fileNameBox{};
    UILabel* m_pathLabel{};
    UIButton* m_filterButton{};
    UIButton* m_okButton{};

    std::wstring m_selectedFile;
    std::vector<std::wstring> m_selectedFiles;
    std::wstring m_selectedFolder;
    bool m_multiSelect{};
    std::wstring m_defaultName;
};

/**
 * @brief  Open file dialog with filters, multi-select and navigation.
 */
class UIOpenFileDialog final : public FileDialogBase {
public:
    UIOpenFileDialog(FileSystem::FileSystemService& fs,
                     std::wstring_view title = L"Open",
                     std::wstring_view initialDir = {},
                     std::wstring_view filters = L"All Files (*.*)|*.*",
                     bool multiSelect = false,
                     bool modal = true) noexcept;

    [[nodiscard]] const std::wstring& GetSelectedFile() const noexcept { return m_selectedFile; }
    [[nodiscard]] const std::vector<std::wstring>& GetSelectedFiles() const noexcept { return m_selectedFiles; }
};

/**
 * @brief  Save file dialog with a default file name and extension logic.
 */
class UISaveFileDialog final : public FileDialogBase {
public:
    UISaveFileDialog(FileSystem::FileSystemService& fs,
                     std::wstring_view title = L"Save As",
                     std::wstring_view initialDir = {},
                     std::wstring_view defaultName = {},
                     std::wstring_view filters = L"All Files (*.*)|*.*",
                     bool modal = true) noexcept;

    [[nodiscard]] const std::wstring& GetSelectedFile() const noexcept { return m_selectedFile; }
};

/**
 * @brief  Folder selection dialog with breadcrumb path and navigation.
 */
class UIFolderDialog final : public FileDialogBase {
public:
    UIFolderDialog(FileSystem::FileSystemService& fs,
                   std::wstring_view title = L"Select Folder",
                   std::wstring_view initialDir = {},
                   bool modal = true) noexcept;

    [[nodiscard]] const std::wstring& GetSelectedFolder() const noexcept { return m_selectedFolder; }
};

} // namespace DragonOS::DragonUI
