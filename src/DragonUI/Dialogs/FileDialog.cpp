#include <DragonUI/Dialogs/FileDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cwctype>
#include <utility>

namespace DragonOS::DragonUI {

// ── Static helpers ──────────────────────────────────────────────────────

std::vector<FileDialogFilter> FileDialogBase::ParseFilters(std::wstring_view spec) noexcept
{
    std::vector<FileDialogFilter> filters;

    std::vector<std::wstring> parts;
    size_t start = 0;
    for (size_t i = 0; i <= spec.size(); ++i)
    {
        if (i == spec.size() || spec[i] == L'|')
        {
            parts.emplace_back(spec.substr(start, i - start));
            start = i + 1;
        }
    }

    for (size_t i = 0; i + 1 < parts.size(); i += 2)
    {
        FileDialogFilter filter;
        filter.display = parts[i];

        std::wstring_view pats = parts[i + 1];
        size_t pStart = 0;
        for (size_t j = 0; j <= pats.size(); ++j)
        {
            if (j == pats.size() || pats[j] == L';')
            {
                auto pat = pats.substr(pStart, j - pStart);
                if (!pat.empty())
                    filter.patterns.emplace_back(pat);
                pStart = j + 1;
            }
        }

        if (filter.patterns.empty())
            filter.patterns.emplace_back(L"*.*");
        filters.push_back(std::move(filter));
    }

    if (filters.empty())
        filters.push_back({L"All Files (*.*)", {L"*.*"}});

    return filters;
}

bool FileDialogBase::WildcardMatch(std::wstring_view pattern, std::wstring_view name) noexcept
{
    size_t p = 0;
    size_t n = 0;
    size_t star = std::wstring_view::npos;
    size_t starN = 0;

    while (n < name.size())
    {
        if (p < pattern.size() &&
            (pattern[p] == L'?' || std::towlower(pattern[p]) == std::towlower(name[n])))
        {
            ++p;
            ++n;
        }
        else if (p < pattern.size() && pattern[p] == L'*')
        {
            star = p++;
            starN = n;
        }
        else if (star != std::wstring_view::npos)
        {
            p = star + 1;
            n = ++starN;
        }
        else
        {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == L'*')
        ++p;
    return p == pattern.size();
}

std::wstring FileDialogBase::NormalizeFilterPattern(std::wstring_view pattern) const noexcept
{
    std::wstring result(pattern);
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
    return result;
}

// ── Construction ────────────────────────────────────────────────────────

FileDialogBase::FileDialogBase(
    FileSystem::FileSystemService& fs,
    std::wstring_view title,
    std::wstring_view initialDir,
    FileDialogMode mode,
    std::wstring_view filters,
    bool modal) noexcept
    : UIDialog(title, modal)
    , m_fs(fs)
    , m_mode(mode)
    , m_source(std::make_shared<FileDialogSource>())
{
    m_filters = ParseFilters(filters);

    SetResizable(true);
    SetCanDrag(true);
    SetSize(640.0f, 430.0f);
    SetMinSize(440.0f, 300.0f);

    BuildBrowser();

    std::wstring start = initialDir.empty() ? std::wstring() : std::wstring(initialDir);
    if (start.empty())
        start = m_fs.GetKnownFolderPath(FileSystem::KnownFolder::Documents);
    if (start.empty())
    {
        auto drives = m_fs.GetLogicalDrives();
        if (!drives.empty())
            start = drives.front();
    }
    NavigateTo(start);
}

void FileDialogBase::BuildBrowser() noexcept
{
    auto grid = std::make_unique<UIGrid>();
    grid->AddRow(GridLength::Pixel(34.0f)); // toolbar
    grid->AddRow(GridLength::Star(1.0f));   // file list
    grid->AddRow(GridLength::Pixel(44.0f)); // bottom row
    grid->SetRowSpacing(8.0f);

    // ── Toolbar row ────────────────────────────────────────────────────
    auto toolbar = std::make_unique<UIGrid>();
    toolbar->AddColumn(GridLength::Auto());
    toolbar->AddColumn(GridLength::Star(1.0f));
    toolbar->SetColumnSpacing(8.0f);

    auto navRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    navRow->SetSpacing(6.0f);

    auto upBtn = std::make_unique<UIButton>(L"Up");
    upBtn->SetIcon(L'\u2191');
    upBtn->SetOnClick([this](UIButton&) noexcept {
        auto parent = FileSystem::FileSystemService::GetParentPath(m_currentDir);
        if (!parent.empty() && parent != m_currentDir)
            NavigateTo(parent);
    });
    navRow->AddChild(std::move(upBtn));

    auto homeBtn = std::make_unique<UIButton>(L"Home");
    homeBtn->SetIcon(L'\u2302');
    homeBtn->SetOnClick([this](UIButton&) noexcept {
        NavigateTo(m_fs.GetKnownFolderPath(FileSystem::KnownFolder::Documents));
    });
    navRow->AddChild(std::move(homeBtn));

    auto refreshBtn = std::make_unique<UIButton>();
    refreshBtn->SetIcon(L'\u27F3');
    refreshBtn->SetOnClick([this](UIButton&) noexcept { RefreshList(); });
    navRow->AddChild(std::move(refreshBtn));

    auto pathLabel = std::make_unique<UILabel>();
    pathLabel->SetWordWrap(false);
    pathLabel->SetAutoSize(false);
    pathLabel->SetTextColor(Theme::SemanticColor::TextSecondary);
    pathLabel->SetVerticalAlignment(Alignment::Center);
    auto pathRaw = pathLabel.get();

    auto navRaw = navRow.get();
    toolbar->AddChild(std::move(navRow));
    toolbar->AddChild(std::move(pathLabel));
    toolbar->SetChildPosition(*navRaw, 0, 0);
    toolbar->SetChildPosition(*pathRaw, 0, 1);

    // ── File list ──────────────────────────────────────────────────────
    auto list = std::make_unique<UIListView>();
    list->SetMode(ListViewMode::Details);
    list->SetItemHeight(24.0f);
    list->SetHeaderVisible(true);

    auto sizeProvider = [](const std::any& value) -> std::wstring {
        const auto& entry = std::any_cast<const FileSystem::FileEntry&>(value);
        return entry.IsDirectory()
                   ? L""
                   : FileSystem::FileSystemService::FormatFileSize(entry.size);
    };
    auto dateProvider = [](const std::any& value) -> std::wstring {
        return FileSystem::FileSystemService::FormatDateTime(
            std::any_cast<const FileSystem::FileEntry&>(value).lastModified);
    };

    list->AddColumn({L"Name", 280.0f, nullptr, nullptr, false});
    list->AddColumn({L"Size", 90.0f, sizeProvider, nullptr, false});
    list->AddColumn({L"Date Modified", 150.0f, dateProvider, nullptr, false});

    list->SetItemSource(m_source);
    list->SetPrimaryTextProvider([](const std::any& value) -> std::wstring {
        return std::any_cast<const FileSystem::FileEntry&>(value).name;
    });
    list->SetPrimaryIconProvider([](const std::any& value) -> uint32_t {
        return std::any_cast<const FileSystem::FileEntry&>(value).IsDirectory()
                   ? 0x1F4C1u
                   : 0x1F4C4u;
    });
    list->SetGlobalSortComparer([](const std::any& a, const std::any& b) -> int {
        const auto& ea = std::any_cast<const FileSystem::FileEntry&>(a);
        const auto& eb = std::any_cast<const FileSystem::FileEntry&>(b);
        if (ea.IsDirectory() != eb.IsDirectory())
            return ea.IsDirectory() ? -1 : 1;
        std::wstring na = ea.name;
        std::wstring nb = eb.name;
        std::transform(na.begin(), na.end(), na.begin(),
            [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
        std::transform(nb.begin(), nb.end(), nb.begin(),
            [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
        return na.compare(nb);
    });
    list->SetOnSelectionChanged([this](const SelectionManager&) noexcept {
        OnSelectionChanged();
    });
    list->SetOnItemActivated([this](UIListView&, int64_t index) noexcept {
        OnItemActivated(index);
    });
    auto listRaw = list.get();
    grid->AddChild(std::move(list));

    // ── Bottom row ─────────────────────────────────────────────────────
    auto bottom = std::make_unique<UIGrid>();
    bottom->AddColumn(GridLength::Auto());
    bottom->AddColumn(GridLength::Star(1.0f));
    bottom->SetColumnSpacing(8.0f);

    auto filterBtn = std::make_unique<UIButton>(m_filters.empty() ? L"Filter" : m_filters.front().display);
    filterBtn->SetOnClick([this](UIButton&) noexcept { CycleFilter(); });
    auto filterRaw = filterBtn.get();
    m_filterButton = filterRaw;
    bottom->AddChild(std::move(filterBtn));

    if (m_mode == FileDialogMode::Folder)
    {
        auto box = std::make_unique<UITextBox>(L"");
        box->SetReadOnly(true);
        auto boxRaw = box.get();
        m_fileNameBox = boxRaw;
        bottom->AddChild(std::move(box));
    }
    else
    {
        auto box = std::make_unique<UITextBox>();
        auto boxRaw = box.get();
        m_fileNameBox = boxRaw;
        bottom->AddChild(std::move(box));
    }

    auto bottomRaw = bottom.get();
    grid->AddChild(std::move(bottom));
    grid->SetChildPosition(*toolbar.get(), 0, 0);
    grid->SetChildPosition(*listRaw, 1, 0);
    grid->SetChildPosition(*bottomRaw, 2, 0);

    AddContent(std::move(grid));

    // ── Dialog buttons ─────────────────────────────────────────────────
    m_okButton = AddButton(L"OK", [this](UIButton&) noexcept {
        Commit();
    });
    UIButton* cancel = AddButton(L"Cancel", DialogResult::Cancel);
    SetDefaultButton(m_okButton);
    SetCancelButton(cancel);

    m_list = listRaw;
    m_pathLabel = pathRaw;

    ApplyFilter();
    m_list->SortItems();
}

// ── Navigation ──────────────────────────────────────────────────────────

void FileDialogBase::NavigateTo(const std::wstring& path) noexcept
{
    if (path.empty())
        return;

    auto result = m_fs.ListDirectory(path);
    if (!result.success)
        return;

    m_currentDir = path;
    m_entries = std::move(result.entries);
    m_source->SetEntries(&m_entries);

    if (m_list)
    {
        m_list->GetSelection().Clear();
        m_list->Refresh();
        m_list->SetScrollOffset(0.0f);
        m_list->SortItems();
    }

    if (m_pathLabel)
        m_pathLabel->SetText(path);

    if (m_mode == FileDialogMode::Folder && m_fileNameBox)
        m_fileNameBox->SetText(path);
}

void FileDialogBase::RefreshList() noexcept
{
    if (!m_currentDir.empty())
        NavigateTo(m_currentDir);
}

// ── Filtering ───────────────────────────────────────────────────────────

void FileDialogBase::ApplyFilter() noexcept
{
    if (!m_list)
        return;

    if (m_mode == FileDialogMode::Folder)
    {
        m_list->ClearFilter();
        return;
    }

    m_list->SetFilter([this](const std::any& value) -> bool {
        return MatchesActiveFilter(std::any_cast<const FileSystem::FileEntry&>(value));
    });
}

void FileDialogBase::CycleFilter() noexcept
{
    if (m_filters.empty())
        return;

    m_filterIndex = (m_filterIndex + 1) % m_filters.size();
    ApplyFilter();
    if (m_filterButton)
        m_filterButton->SetText(m_filters[m_filterIndex].display);
}

bool FileDialogBase::MatchesActiveFilter(const FileSystem::FileEntry& entry) const noexcept
{
    if (entry.IsDirectory())
        return true;
    if (m_filters.empty())
        return true;

    const auto& filter = m_filters[m_filterIndex % m_filters.size()];
    for (const auto& pattern : filter.patterns)
    {
        if (WildcardMatch(pattern, entry.name))
            return true;
    }
    return false;
}

// ── Entry lookup ────────────────────────────────────────────────────────

FileSystem::FileEntry FileDialogBase::EntryAt(int64_t displayIndex) const noexcept
{
    if (!m_list || displayIndex < 0)
        return {};

    std::any value = m_list->GetItemAt(displayIndex);
    if (!value.has_value())
        return {};

    try
    {
        return std::any_cast<FileSystem::FileEntry>(value);
    }
    catch (const std::bad_any_cast&)
    {
        return {};
    }
}

std::wstring FileDialogBase::SelectedPathFromEntry(int64_t displayIndex) const noexcept
{
    auto entry = EntryAt(displayIndex);
    return entry.name.empty() ? std::wstring() : entry.fullPath;
}

// ── Selection / activation ──────────────────────────────────────────────

void FileDialogBase::OnSelectionChanged() noexcept
{
    if (!m_list)
        return;

    const int64_t index = m_list->GetSelection().GetFirstSelected();
    if (index < 0)
        return;

    auto entry = EntryAt(index);
    if (entry.name.empty())
        return;

    if (m_mode == FileDialogMode::Open || m_mode == FileDialogMode::Save)
    {
        if (m_fileNameBox && !entry.IsDirectory())
            m_fileNameBox->SetText(entry.name);
    }
    else
    {
        if (entry.IsDirectory())
            m_selectedFolder = entry.fullPath;
    }
}

void FileDialogBase::OnItemActivated(int64_t displayIndex) noexcept
{
    auto entry = EntryAt(displayIndex);
    if (entry.name.empty())
        return;

    if (entry.IsDirectory())
    {
        NavigateTo(entry.fullPath);
        return;
    }

    if (m_mode == FileDialogMode::Open || m_mode == FileDialogMode::Save)
    {
        if (m_fileNameBox)
            m_fileNameBox->SetText(entry.name);

        if (m_mode == FileDialogMode::Open)
        {
            m_selectedFile = entry.fullPath;
            Close(DialogResult::OK);
        }
    }
}

// ── Commit ──────────────────────────────────────────────────────────────

void FileDialogBase::Commit() noexcept
{
    std::wstring name = m_fileNameBox ? m_fileNameBox->GetText() : std::wstring();

    if (m_mode == FileDialogMode::Open)
    {
        m_selectedFiles.clear();
        if (m_multiSelect && m_list)
        {
            for (int64_t index : m_list->GetSelection().GetSelectedIndices())
            {
                auto entry = EntryAt(index);
                if (!entry.IsDirectory() && !entry.fullPath.empty())
                    m_selectedFiles.push_back(entry.fullPath);
            }
            if (!m_selectedFiles.empty())
            {
                m_selectedFile = m_selectedFiles.front();
                Close(DialogResult::OK);
                return;
            }
        }

        if (name.empty())
        {
            const int64_t index = m_list ? m_list->GetSelection().GetFirstSelected() : -1;
            auto entry = EntryAt(index);
            if (entry.IsDirectory())
            {
                if (!entry.fullPath.empty())
                    NavigateTo(entry.fullPath);
                return;
            }
            name = entry.name;
        }

        m_selectedFile = FileSystem::FileSystemService::Combine(m_currentDir, name);
        if (!m_selectedFile.empty())
            m_selectedFiles.push_back(m_selectedFile);
        Close(DialogResult::OK);
        return;
    }

    if (m_mode == FileDialogMode::Save)
    {
        if (name.empty())
            name = m_defaultName;

        if (!name.empty() && !m_filters.empty())
        {
            const auto& filter = m_filters[m_filterIndex % m_filters.size()];
            if (!filter.patterns.empty())
            {
                std::wstring ext;
                for (wchar_t ch : filter.patterns.front())
                {
                    if (ch == L'*' || ch == L'?' || ch == L'.')
                    {
                        if (ch != L'*' && ch != L'?')
                            ext.push_back(ch);
                    }
                    else
                    {
                        break;
                    }
                }
                if (ext == L".")
                    ext.clear();
                if (!ext.empty() &&
                    FileSystem::FileSystemService::GetExtension(name).empty())
                {
                    name += ext;
                }
            }
        }

        m_selectedFile = FileSystem::FileSystemService::Combine(m_currentDir, name);
        Close(DialogResult::OK);
        return;
    }

    // Folder mode.
    if (m_selectedFolder.empty())
        m_selectedFolder = m_currentDir;
    Close(DialogResult::OK);
}

// ── Concrete dialogs ────────────────────────────────────────────────────

UIOpenFileDialog::UIOpenFileDialog(
    FileSystem::FileSystemService& fs,
    std::wstring_view title,
    std::wstring_view initialDir,
    std::wstring_view filters,
    bool multiSelect,
    bool modal) noexcept
    : FileDialogBase(fs, title, initialDir, FileDialogMode::Open, filters, modal)
{
    m_multiSelect = multiSelect;
    if (m_multiSelect && m_list)
        m_list->GetSelection().SetMode(SelectionMode::Multi);
}

UISaveFileDialog::UISaveFileDialog(
    FileSystem::FileSystemService& fs,
    std::wstring_view title,
    std::wstring_view initialDir,
    std::wstring_view defaultName,
    std::wstring_view filters,
    bool modal) noexcept
    : FileDialogBase(fs, title, initialDir, FileDialogMode::Save, filters, modal)
{
    m_defaultName = defaultName;
    if (m_fileNameBox && !m_defaultName.empty())
        m_fileNameBox->SetText(m_defaultName);
}

UIFolderDialog::UIFolderDialog(
    FileSystem::FileSystemService& fs,
    std::wstring_view title,
    std::wstring_view initialDir,
    bool modal) noexcept
    : FileDialogBase(fs, title, initialDir, FileDialogMode::Folder, {}, modal)
{
}

} // namespace DragonOS::DragonUI
