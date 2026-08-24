#include <Explorer2/FileListPane.hpp>
#include <Explorer2/ExplorerViewModel.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/DataBinding/CollectionViewSource.hpp>
#include <DragonUI/Controls/ListView.hpp>

#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction / binding
// ============================================================================

FileListPane::FileListPane() noexcept
{
    m_root->SetAccessibleName(L"File list");
    m_root->SetAutomationId(L"Explorer2/FileList");

    auto list = std::make_unique<UIListView>();
    m_list = list.get();
    m_list->SetAccessibleName(L"Files and folders");
    m_list->SetAutomationId(L"Explorer2/FileList/Items");
    m_list->SetItemHeight(26.0f);
    m_list->SetTileExtent(110.0f, 96.0f);
    m_list->GetSelection().SetMode(SelectionMode::Multi);

    ConfigureColumns();

    m_root->AddChild(std::move(list));
    m_root->SetChildDock(*m_list, Dock::Fill);
    m_root->SetLastChildFill(true);

    InstallProviders();
}

void FileListPane::ConfigureColumns() noexcept
{
    m_list->ClearColumns();

    // Name column - always present
    UIListView::Column name;
    name.title = L"Name";
    name.width = 240.0f;
    // No-op comparers: the view model owns ordering (grouping-aware); the
    // control-level sort is only used to drive the header sort arrow.
    name.comparer = [](const std::any&, const std::any&) { return 0; };
    m_list->AddColumn(std::move(name));

    // Size column
    UIListView::Column size;
    size.title = L"Size";
    size.width = 100.0f;
    size.comparer = [](const std::any&, const std::any&) { return 0; };
    m_list->AddColumn(std::move(size));

    // Type column
    UIListView::Column type;
    type.title = L"Type";
    type.width = 160.0f;
    type.comparer = [](const std::any&, const std::any&) { return 0; };
    m_list->AddColumn(std::move(type));

    // Date modified column
    UIListView::Column date;
    date.title = L"Date modified";
    date.width = 170.0f;
    date.comparer = [](const std::any&, const std::any&) { return 0; };
    m_list->AddColumn(std::move(date));
}

void FileListPane::InstallProviders() noexcept
{
    // Primary text provider - entry name or group header label
    m_list->SetPrimaryTextProvider([](const std::any& value) -> std::wstring {
        if (const auto* row = std::any_cast<FileRow>(&value))
        {
            return row->IsHeader() ? row->groupLabel : row->entry.name;
        }
        return {};
    });

    // Primary icon provider - glyph per entry
    m_list->SetPrimaryIconProvider([this](const std::any& value) -> uint32_t {
        if (const auto* row = std::any_cast<FileRow>(&value))
        {
            if (row->IsHeader()) { return IconLibrary::ChevronDown; }
            return IconLibrary::GlyphFor(row->entry);
        }
        return 0;
    });

    // On item activated - route to view model / window callback
    m_list->SetOnItemActivated([this](UIListView& list, int64_t index) {
        const std::any value = list.GetItemAt(index);
        const auto* row = std::any_cast<FileRow>(&value);
        if (!row) { return; }

        if (row->IsHeader())
        {
            if (m_viewModel)
            {
                m_viewModel->ToggleGroupCollapsed(row->groupLabel);
            }
            return;
        }
        if (m_onActivate)
        {
            m_onActivate(row->entry);
        }
    });

    // On selection changed
    m_list->SetOnSelectionChanged([this](const SelectionManager&) {
        if (m_onSelectionChanged)
        {
            m_onSelectionChanged();
        }
    });

    // Item renderer - custom drawing with icons, columns, groups, tiles
    m_list->SetItemRenderer(
        [this](RenderContext& ctx, const LayoutSlot& slot,
               const std::any& value, const ItemVisualState& state) {
            const auto* row = std::any_cast<FileRow>(&value);
            if (!row) { return; }

            const D2D1_RECT_F rect{ slot.x, slot.y, slot.x + slot.width, slot.y + slot.height };

            if (row->IsHeader())
            {
                // Group header band
                ctx.FillRectangle(rect, Theme::SemanticColor::WindowTitleBar, 0.85f);

                const bool collapsed =
                    m_viewModel && m_viewModel->IsGroupCollapsed(row->groupLabel);
                const uint32_t chevron =
                    collapsed ? IconLibrary::ChevronRight : IconLibrary::ChevronDown;

                const std::wstring chevronText = CodepointToUtf16(chevron);
                ctx.DrawText(chevronText, { rect.left + 6.0f, rect.top + 2.0f,
                                            rect.left + 24.0f, rect.bottom },
                             Theme::SemanticColor::Accent);

                wchar_t count[32];
                (void)swprintf_s(count, L"(%zu)", row->groupCount);
                const std::wstring countText = count;

                const D2D1_RECT_F labelRect{
                    rect.left + 28.0f, rect.top + 2.0f,
                    rect.right - 60.0f, rect.bottom - 2.0f };
                ctx.DrawText(row->groupLabel, labelRect, Theme::SemanticColor::TextPrimary);

                const auto countSize = ctx.MeasureText(countText);
                const D2D1_RECT_F countRect{
                    rect.right - countSize.width - 12.0f, rect.top + 2.0f,
                    rect.right - 8.0f, rect.bottom - 2.0f };
                ctx.DrawText(countText, countRect, Theme::SemanticColor::TextSecondary);
                return;
            }

            // Entry rows
            const FileSystem::FileEntry& entry = row->entry;
            const uint32_t glyph = IconLibrary::GlyphFor(entry);
            const std::wstring iconText = CodepointToUtf16(glyph);
            const Theme::SemanticColor textColor = state.selected
                ? Theme::SemanticColor::TextPrimary
                : entry.IsHidden()
                    ? Theme::SemanticColor::TextSecondary
                    : Theme::SemanticColor::TextPrimary;

            const auto mode = m_list->GetMode();
            const float iconSize = mode == ListViewMode::List ? 18.0f : 20.0f;
            const auto iconMetrics = ctx.MeasureText(iconText, iconSize * 4.0f);

            if (mode == ListViewMode::Tile)
            {
                // Large centered icon + two text lines
                const float bigIcon = 34.0f;
                const auto metrics = ctx.MeasureText(iconText, bigIcon * 4.0f);
                const float iconX = rect.left + ((rect.right - rect.left) - metrics.width) * 0.5f;
                ctx.DrawText(iconText,
                             { iconX, rect.top + 6.0f, iconX + metrics.width, rect.top + 6.0f + metrics.height },
                             Theme::SemanticColor::TextSecondary);

                const D2D1_RECT_F nameRect{ rect.left + 2.0f, rect.top + bigIcon + 8.0f,
                                            rect.right - 2.0f, rect.top + bigIcon + 24.0f };
                ctx.DrawText(entry.name, nameRect, textColor);

                const D2D1_RECT_F typeRect{ rect.left + 2.0f, rect.top + bigIcon + 24.0f,
                                            rect.right - 2.0f, rect.bottom - 2.0f };
                ctx.DrawText(GetTypeDisplayName(entry), typeRect, Theme::SemanticColor::TextSecondary);
                return;
            }

            // Details / List: leading icon then columns
            float tx = rect.left + 8.0f;
            {
                const D2D1_RECT_F iconRect{
                    tx, rect.top + (rect.bottom - rect.top - iconMetrics.height) * 0.5f,
                    tx + iconMetrics.width,
                    rect.top + (rect.bottom - rect.top - iconMetrics.height) * 0.5f + iconMetrics.height };
                ctx.DrawText(iconText, iconRect, state.selected
                    ? Theme::SemanticColor::TextPrimary
                    : Theme::SemanticColor::TextSecondary);
                tx += iconMetrics.width + 6.0f;
            }

            const auto& columns = m_list->GetColumns();

            if (mode == ListViewMode::List || columns.empty())
            {
                const D2D1_RECT_F textRect{ tx, rect.top + 2.0f, rect.right - 8.0f, rect.bottom - 2.0f };
                ctx.DrawText(entry.name, textRect, textColor);
                return;
            }

            // Column 0 — Name
            {
                const float colW = columns[0].width;
                const D2D1_RECT_F cell{ tx, rect.top + 2.0f,
                                        rect.left + colW - 4.0f, rect.bottom - 2.0f };
                ctx.DrawText(entry.name, cell, textColor);
            }

            for (size_t c = 1; c < columns.size(); ++c)
            {
                float colX = rect.left;
                for (size_t p = 0; p < c; ++p)
                {
                    colX += columns[p].width;
                }

                std::wstring text;
                switch (c)
                {
                case 1:
                    text = entry.IsDirectory() ? L"" : FileSystem::FileSystemService::FormatFileSize(entry.size);
                    break;
                case 2:
                    text = GetTypeDisplayName(entry);
                    break;
                case 3:
                    text = FileSystem::FileSystemService::FormatDateTime(entry.lastModified);
                    break;
                default:
                    break;
                }

                const D2D1_RECT_F cell{ colX + 8.0f, rect.top + 2.0f,
                                        colX + columns[c].width - 4.0f, rect.bottom - 2.0f };
                ctx.DrawText(text, cell, state.selected
                    ? Theme::SemanticColor::TextPrimary
                    : Theme::SemanticColor::TextSecondary);
            }
        });
}

// ============================================================================
//  Presentation
// ============================================================================

void FileListPane::ApplyViewMode(ViewMode mode) noexcept
{
    switch (mode)
    {
    case ViewMode::Details:
        m_list->SetMode(ListViewMode::Details);
        m_list->SetHeaderVisible(true);
        break;
    case ViewMode::List:
        m_list->SetMode(ListViewMode::List);
        break;
    case ViewMode::Tiles:
        m_list->SetMode(ListViewMode::Tile);
        break;
    }
    m_list->Refresh();
    m_lastAppliedMode = mode;
}

void FileListPane::NotifySortChanged() noexcept
{
    RefreshSortIndicators();
    if (m_onSortChanged)
    {
        m_onSortChanged();
    }
}

void FileListPane::RefreshSortIndicators() noexcept
{
    if (!m_viewModel)
    {
        return;
    }

    size_t column = 0;
    switch (m_viewModel->GetSortKey())
    {
    case SortKey::Size:         column = 1; break;
    case SortKey::Type:         column = 2; break;
    case SortKey::DateModified: column = 3; break;
    case SortKey::Name:         column = 0; break;
    }

    m_list->SortByColumn(column, ToControlDirection(m_viewModel->GetSortDir()));
}

// ============================================================================
//  Selection helpers
// ============================================================================

std::vector<const FileSystem::FileEntry*> FileListPane::GetSelectedEntries() const noexcept
{
    std::vector<const FileSystem::FileEntry*> out;
    if (!m_list || !m_source) { return out; }

    out.reserve(static_cast<size_t>(m_list->GetSelection().GetSelectedCount()));
    for (const int64_t index : m_list->GetSelection().GetSelectedIndices())
    {
        const std::any value = m_source->GetCollection()->Get(index);
        if (const auto* row = std::any_cast<FileRow>(&value); row && !row->IsHeader())
        {
            out.push_back(&row->entry);
        }
    }
    return out;
}

const FileSystem::FileEntry* FileListPane::GetSingleSelectedEntry() const noexcept
{
    const auto entries = GetSelectedEntries();
    return entries.size() == 1 ? entries.front() : nullptr;
}

void FileListPane::ClearSelection() noexcept
{
    if (m_list)
    {
        m_list->GetSelection().Clear();
        m_list->InvalidateVisual();
    }
}

void FileListPane::SelectAllEntries() noexcept
{
    if (!m_list || !m_source)
    {
        return;
    }

    auto& selection = m_list->GetSelection();
    selection.Clear();
    const auto& rows = *m_source->GetCollection();
    for (int64_t i = 0; i < rows.GetCount(); ++i)
    {
        if (!rows.Get(i).IsHeader())
        {
            selection.Toggle(i);
        }
    }
    m_list->InvalidateVisual();
    if (m_onSelectionChanged)
    {
        m_onSelectionChanged();
    }
}

// ============================================================================
//  Hit testing / header interception
// ============================================================================

const FileSystem::FileEntry* FileListPane::GetEntryAtPoint(float x, float y) const noexcept
{
    const FileRow* row = GetRowAtPoint(x, y);
    return (row && !row->IsHeader()) ? &row->entry : nullptr;
}

const FileRow* FileListPane::GetRowAtPoint(float x, float y) const noexcept
{
    if (!m_list || !m_source)
    {
        return nullptr;
    }
    const int64_t index = m_list->HitTestItemAt(x, y);
    if (index < 0)
    {
        return nullptr;
    }
    const std::any value = m_source->GetCollection()->Get(index);
    return std::any_cast<FileRow>(&value);
}

bool FileListPane::HandleHeaderClick(float x, float y) noexcept
{
    if (!m_list || !m_viewModel)
    {
        return false;
    }
    if (m_list->GetMode() != ListViewMode::Details || m_list->GetColumns().empty())
    {
        return false;
    }

    const float localY = y - m_list->GetY();
    if (localY < 0.0f || localY >= 26.0f)
    {
        return false;
    }

    const int column = ColumnIndexFromLocalX(x - m_list->GetX());
    if (column < 0)
    {
        return false;
    }

    m_viewModel->ToggleSort(SortKeyForColumn(static_cast<size_t>(column)));
    RefreshSortIndicators();
    if (m_onSortChanged)
    {
        m_onSortChanged();
    }
    return true;
}

int FileListPane::ColumnIndexFromLocalX(float localX) const noexcept
{
    const auto& columns = m_list->GetColumns();
    const float contentWidth = m_list->GetWidth() - 10.0f; // ScrollBarWidth
    if (localX < 0.0f || localX >= contentWidth)
    {
        return -1;
    }

    float acc = 0.0f;
    for (size_t c = 0; c < columns.size(); ++c)
    {
        acc += columns[c].width;
        if (localX < acc)
        {
            return static_cast<int>(c);
        }
    }
    return -1;
}

DragonUI::SortDirection FileListPane::ToControlDirection(SortDir dir) noexcept
{
    return dir == SortDir::Descending ? DragonUI::SortDirection::Descending
                                      : DragonUI::SortDirection::Ascending;
}

// ============================================================================
//  Misc
// ============================================================================

void FileListPane::RevealEntry(const std::wstring& fullPath) noexcept
{
    if (!m_source)
    {
        return;
    }
    const auto& rows = *m_source->GetCollection();
    for (int64_t i = 0; i < rows.GetCount(); ++i)
    {
        if (!rows.Get(i).IsHeader() && rows.Get(i).entry.fullPath == fullPath)
        {
            m_list->ScrollToItem(i);
            return;
        }
    }
}

void FileListPane::SetFocusToList() noexcept
{
    // Focus is assigned by the owning window through its FocusManager;
    // the pane only exposes the list for that purpose.
}

// ============================================================================
//  Shared helpers (anonymous namespace)
// ============================================================================

namespace {

[[nodiscard]] SortKey SortKeyForColumn(size_t column) noexcept
{
    switch (column)
    {
    case 1:  return SortKey::Size;
    case 2:  return SortKey::Type;
    case 3:  return SortKey::DateModified;
    default: return SortKey::Name;
    }
}

} // namespace

} // namespace DragonOS::Explorer2