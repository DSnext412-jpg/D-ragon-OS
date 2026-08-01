#include <DragonUI/Demo/DataControlsDemo.hpp>

#include <DragonUI/Core/ItemTemplate.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <DragonUI/Core/VirtualItemSource.hpp>
#include <DragonUI/Core/WindowHost.hpp>
#include <DragonUI/Dialogs/DialogManager.hpp>
#include <DragonUI/Dialogs/UIMessageBox.hpp>
#include <DragonUI/Dialogs/FileDialog.hpp>
#include <DragonUI/Dialogs/UIColorDialog.hpp>
#include <DragonUI/Dialogs/UIFontDialog.hpp>
#include <DragonUI/Dialogs/UIProgressDialog.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/TreeView.hpp>
#include <DragonUI/Controls/GridView.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/Separator.hpp>

#include <FileSystem/FileSystemService.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI::Demo {

// ============================================================================
//  Plugin Manager demo data
// ============================================================================

namespace {

struct PluginRow {
    std::wstring name;
    std::wstring version;
    std::wstring author;
    bool enabled{ true };
    std::wstring status;
};

class PluginRowSource final : public VirtualItemSource {
public:
    [[nodiscard]] int64_t GetCount() const noexcept override
    {
        return static_cast<int64_t>(rows.size());
    }

    [[nodiscard]] std::any GetItem(int64_t index) const override
    {
        if (index < 0 || index >= static_cast<int64_t>(rows.size()))
            return {};
        return rows[static_cast<size_t>(index)];
    }

    std::vector<PluginRow> rows;
};

std::vector<PluginRow> MakeSamplePlugins()
{
    const struct { const wchar_t* name; const wchar_t* version; const wchar_t* author; bool enabled; const wchar_t* status; } defs[] = {
        { L"File Explorer Extensions", L"1.4.2",  L"D'ragon Team", true,  L"Running" },
        { L"Package Manager Core",     L"2.1.0",  L"D'ragon Team", true,  L"Running" },
        { L"System Monitor",           L"0.9.3",  L"D'ragon Team", true,  L"Running" },
        { L"Network Diagnostics",      L"1.0.0",  L"Community",    false, L"Disabled" },
        { L"Terminal Theme Pack",      L"0.4.1",  L"Community",    true,  L"Running" },
        { L"Old File Handler",         L"0.2.0",  L"Legacy Labs",  false, L"Incompatible" },
        { L"Notification Bridge",      L"3.0.1",  L"D'ragon Team", true,  L"Running" },
        { L"Wallpaper Service",        L"1.2.7",  L"D'ragon Team", true,  L"Running" },
        { L"Gamepad Input",            L"0.8.0",  L"Community",    true,  L"Idle" },
        { L"Crash Reporter",           L"1.1.1",  L"D'ragon Team", false, L"Error" },
    };

    std::vector<PluginRow> result;
    result.reserve(10);
    for (const auto& d : defs)
        result.push_back({ d.name, d.version, d.author, d.enabled, d.status });
    return result;
}

// ============================================================================
//  File list demo data (virtualization stress test)
// ============================================================================

class FileRowSource final : public VirtualItemSource {
public:
    explicit FileRowSource(int64_t count) noexcept
        : m_count(count)
    {
    }

    [[nodiscard]] int64_t GetCount() const noexcept override { return m_count; }

    [[nodiscard]] std::any GetItem(int64_t index) const override
    {
        if (index < 0 || index >= m_count)
            return {};
        return L"Item " + std::to_wstring(index + 1);
    }

private:
    int64_t m_count;
};

uint32_t IconForFile(const std::any& value) noexcept
{
    static const uint32_t glyphs[] = { 0x1F4C1, 0x1F4C4, 0x1F4DD, 0x1F5BC, 0x2699 };
    const auto* text = std::any_cast<std::wstring>(&value);
    const size_t h = text ? std::hash<std::wstring>{}(*text) : 0u;
    return glyphs[h % 5];
}

// ============================================================================
//  Demo layout
// ============================================================================

class Section : public Element {
public:
    explicit Section(std::wstring_view title) noexcept
        : m_title(title)
    {
    }

    void AddChild(std::unique_ptr<Element> child) noexcept
    {
        m_child = std::move(child);
        InvalidateLayout();
    }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override
    {
        if (m_child)
            m_child->Measure({ available.x, available.y, available.width, FLT_MAX });
        const float h = (m_child ? m_child->GetDesiredSize().height : 0.0f) + 34.0f;
        return { (std::min)(320.0f, available.width), h };
    }

    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override
    {
        if (m_child)
        {
            const float childH = m_child->GetDesiredSize().height;
            m_child->Arrange({ finalSlot.x, finalSlot.y + 30.0f, finalSlot.width, childH });
        }
    }

    void Render(RenderContext& ctx) noexcept override
    {
        Element::Render(ctx);
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        ctx.FillRectangle(d2d, Theme::SemanticColor::ControlFill, 0.3f);
        ctx.DrawRectangle(d2d, Theme::SemanticColor::WindowBorder, 1.0f);
        ctx.DrawText(m_title, { d2d.left + 10.0f, d2d.top + 5.0f, d2d.right - 10.0f, d2d.top + 26.0f },
            Theme::SemanticColor::TextSecondary);
        if (m_child)
            m_child->Render(ctx);
    }

private:
    std::wstring m_title;
    std::unique_ptr<Element> m_child;
};

std::unique_ptr<UIGridView> CreatePluginGridView() noexcept
{
    auto source = std::make_shared<PluginRowSource>();
    source->rows = MakeSamplePlugins();

    auto grid = std::make_unique<UIGridView>();
    grid->SetMinSize(0, 240);
    grid->SetRowSource(std::move(source));
    grid->SetRowHeight(26.0f);
    grid->SetAlternatingRowColors(true);

    UIGridView::Column nameColumn;
    nameColumn.title = L"Plugin";
    nameColumn.width = 220.0f;
    nameColumn.getText = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).name;
    };
    grid->AddColumn(std::move(nameColumn));

    UIGridView::Column versionColumn;
    versionColumn.title = L"Version";
    versionColumn.width = 90.0f;
    versionColumn.getText = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).version;
    };
    grid->AddColumn(std::move(versionColumn));

    UIGridView::Column authorColumn;
    authorColumn.title = L"Author";
    authorColumn.width = 170.0f;
    authorColumn.getText = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).author;
    };
    grid->AddColumn(std::move(authorColumn));

    UIGridView::Column enabledColumn;
    enabledColumn.title = L"Enabled";
    enabledColumn.width = 90.0f;
    enabledColumn.getText = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).enabled ? L"Yes" : L"No";
    };
    enabledColumn.getColor = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).enabled
            ? Theme::SemanticColor::Success
            : Theme::SemanticColor::Disabled;
    };
    grid->AddColumn(std::move(enabledColumn));

    UIGridView::Column statusColumn;
    statusColumn.title = L"Status";
    statusColumn.width = 140.0f;
    statusColumn.getText = [](const std::any& v) {
        return std::any_cast<PluginRow>(v).status;
    };
    statusColumn.getColor = [](const std::any& v) {
        const std::wstring& s = std::any_cast<PluginRow>(v).status;
        if (s == L"Running") return Theme::SemanticColor::Success;
        if (s == L"Error") return Theme::SemanticColor::Error;
        if (s == L"Incompatible") return Theme::SemanticColor::Warning;
        return Theme::SemanticColor::TextSecondary;
    };
    grid->AddColumn(std::move(statusColumn));

    return grid;
}

std::unique_ptr<UIListView> CreateFileListView() noexcept
{
    auto list = std::make_unique<UIListView>();
    list->SetMinSize(0, 200);
    list->SetMode(ListViewMode::List);
    list->SetItemHeight(26.0f);
    list->SetItemSource(std::make_shared<FileRowSource>(50000));
    list->SetPrimaryIconProvider(IconForFile);
    list->GetSelection().SetMode(SelectionMode::Multi);
    return list;
}

std::unique_ptr<UITreeView> CreateFolderTreeView() noexcept
{
    auto tree = std::make_unique<UITreeView>();
    tree->SetMinSize(0, 200);

    auto* home = tree->AddRootNode(L"Home", 0x1F3E0);
    auto* desktop = home->AddChild(L"Desktop", 0x1F4BB);
    desktop->AddChild(L"Projects", 0x1F4C1);
    desktop->AddChild(L"Screenshots", 0x1F5BC);
    home->AddChild(L"Documents", 0x1F4C4);
    home->AddChild(L"Downloads", 0x1F4E5);

    auto* system = tree->AddRootNode(L"This PC", 0x1F4BE);
    auto* drives = system->AddChild(L"Drives", 0x1F4BE);
    drives->AddChild(L"C: (System)", 0x1F4C1);
    drives->AddChild(L"D: (Data)", 0x1F4C1);

    auto* network = tree->AddRootNode(L"Network", 0x1F5A7);
    network->SetLeaf(true);

    // Lazy-loading node: children are materialized on first expand.
    auto* lazy = tree->AddRootNode(L"Plugin Packages", 0x1F4E6);
    lazy->SetLazyLoader([](UITreeNode& node) {
        node.AddChild(L"explorer-pack-1.4.2", 0x1F4E6);
        node.AddChild(L"monitor-pack-0.9.3", 0x1F4E6);
        node.AddChild(L"theme-pack-0.4.1", 0x1F4E6);
    });

    tree->SetOnNodeActivated([](UITreeView& tv, UITreeNode& node) {
        (void)tv;
        (void)node;
    });

    return tree;
}

// ============================================================================
//  Dialog framework demo
// ============================================================================

std::wstring DialogResultText(DialogResult result) noexcept
{
    switch (result)
    {
    case DialogResult::OK:     return L"OK";
    case DialogResult::Cancel: return L"Cancel";
    case DialogResult::Yes:    return L"Yes";
    case DialogResult::No:     return L"No";
    case DialogResult::Retry:  return L"Retry";
    case DialogResult::Abort:  return L"Abort";
    case DialogResult::Ignore: return L"Ignore";
    case DialogResult::Close:  return L"Close";
    case DialogResult::None:   return L"None";
    }
    return L"?";
}

/**
 * @brief  Dialog-framework demo root. Owns the FileSystemService it passes
 *         to the file dialogs, wires a status label to every dialog, and
 *         drives the progress dialog from its per-frame Measure pass.
 */
class DialogDemoRoot final : public Container {
public:
    explicit DialogDemoRoot(WindowHost& host) noexcept
        : m_host(host)
        , m_fs(new FileSystem::FileSystemService())
    {
        m_status = new UILabel(L"Pick a dialog to try it out.");
        AddChild(std::move(std::unique_ptr<UILabel>(m_status)));
        AddChild(MakeButton(L"Message Box", [this](UIButton&) noexcept {
            auto box = UIMessageBox::Create(
                L"Information", L"DragonUI dialog framework is working.",
                MessageBoxButtons::OK, MessageBoxIcon::Info);
            Show(std::move(box));
        }));
        AddChild(MakeButton(L"Confirm (Yes / No)", [this](UIButton&) noexcept {
            auto box = UIMessageBox::Create(
                L"Question", L"Would you like to continue?",
                MessageBoxButtons::YesNo, MessageBoxIcon::Question);
            Show(std::move(box));
        }));
        AddChild(MakeButton(L"Open File...", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UIOpenFileDialog>(
                *m_fs, L"Open File",
                m_fs->GetKnownFolderPath(FileSystem::KnownFolder::Documents),
                L"Text Files (*.txt)|*.txt;All Files (*.*)|*.*", false);
            Show(std::move(dlg));
        }));
        AddChild(MakeButton(L"Save File...", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UISaveFileDialog>(
                *m_fs, L"Save As",
                m_fs->GetKnownFolderPath(FileSystem::KnownFolder::Documents),
                L"untitled.txt",
                L"Text Files (*.txt)|*.txt;All Files (*.*)|*.*");
            Show(std::move(dlg));
        }));
        AddChild(MakeButton(L"Select Folder...", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UIFolderDialog>(
                *m_fs, L"Select Folder",
                m_fs->GetKnownFolderPath(FileSystem::KnownFolder::Documents));
            Show(std::move(dlg));
        }));
        AddChild(MakeButton(L"Color Picker...", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UIColorDialog>(
                Theme::ThemeColor::FromRGB(86, 156, 214), L"Select Color");
            Show(std::move(dlg));
        }));
        AddChild(MakeButton(L"Font Picker...", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UIFontDialog>(L"Select Font", L"Segoe UI", 14.0f, false, false);
            Show(std::move(dlg));
        }));
        auto progressBtn = MakeButton(L"Run Progress", [this](UIButton&) noexcept {
            auto dlg = std::make_unique<UIProgressDialog>(L"Working...", false, true);
            m_progress = dlg.get();
            m_start = std::chrono::steady_clock::now();
            m_host.Dialogs().ShowDialog(std::move(dlg), m_host.GetFocusManager());
        });
        m_progressBtn = progressBtn.get();
        AddChild(std::move(progressBtn));
    }

    // ── Layout: stack children vertically with 8px spacing ──────────

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override
    {
        Tick();
        float h = 0.0f;
        float w = 0.0f;
        for (const auto& child : GetChildren())
        {
            child->Measure({ available.x, available.y, available.width, FLT_MAX });
            const auto ds = child->GetDesiredSize();
            h += (h > 0.0f ? 8.0f : 0.0f) + ds.height;
            w = (std::max)(w, ds.width);
        }
        return { (std::min)(320.0f, w), h };
    }

    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override
    {
        float y = finalSlot.y;
        for (const auto& child : GetChildren())
        {
            const auto ds = child->GetDesiredSize();
            child->Arrange({ finalSlot.x, y, finalSlot.width, ds.height });
            y += ds.height + 8.0f;
        }
    }

private:
    static std::unique_ptr<UIButton> MakeButton(
        std::wstring_view text,
        UIButton::ClickCallback onClick) noexcept
    {
        auto btn = std::make_unique<UIButton>(text);
        btn->SetOnClick(std::move(onClick));
        return btn;
    }

    void Show(std::unique_ptr<UIDialog> dlg) noexcept
    {
        dlg->SetOnClosed([this](UIDialog&, DialogResult result) noexcept {
            m_status->SetText(L"Last result: " + DialogResultText(result));
        });
        m_host.Dialogs().ShowDialog(std::move(dlg), m_host.GetFocusManager());
    }

    void Tick() noexcept
    {
        if (!m_progress || !m_progress->IsOpen())
        {
            m_progress = nullptr;
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - m_start).count();
        const float pct = static_cast<float>(std::fmod(elapsed / 6.0, 1.0));
        m_progress->SetProgress(pct);
        m_progress->SetStatus(L"Processing item " + std::to_wstring(static_cast<int>(elapsed * 4.0) % 25));
        if (m_progress->IsCancelled())
        {
            m_progress->SetStatus(L"Cancelled");
            m_progress->Close(DialogResult::Cancel);
        }
        if (elapsed >= 6.0)
            m_progress->Close(DialogResult::OK);
    }

    WindowHost& m_host;
    UILabel* m_status{};
    std::unique_ptr<FileSystem::FileSystemService> m_fs;
    UIButton* m_progressBtn{};
    UIProgressDialog* m_progress{};
    std::chrono::steady_clock::time_point m_start;
};

std::unique_ptr<Element> CreateDialogDemo(WindowHost& host) noexcept
{
    return std::make_unique<DialogDemoRoot>(host);
}

} // namespace

// ============================================================================
//  Public demo entry points
// ============================================================================

std::unique_ptr<Element> PluginManagerDemo::Create() noexcept
{
    auto grid = CreatePluginGridView();
    return grid;
}

std::unique_ptr<Element> DataControlsDemo::Create(WindowHost& host) noexcept
{
    auto root = std::make_unique<UIStackPanel>(Orientation::Vertical);
    root->SetPadding(Thickness(16));
    root->SetSpacing(14);

    auto title = std::make_unique<UILabel>(L"DragonUI Data Controls Demo");
    title->SetTextAlignment(Alignment::Center);
    title->SetMinSize(0, 28);
    root->AddChild(std::move(title));

    // ── 1. UIGridView — Plugin Manager ────────────────────────────────
    {
        auto section = std::make_unique<Section>(L"1. UIGridView — Plugin Manager (sortable columns, resizable headers)");
        section->AddChild(CreatePluginGridView());
        root->AddChild(std::move(section));
    }

    // ── 2. UIListView — Virtualized file list ─────────────────────────
    {
        auto section = std::make_unique<Section>(L"2. UIListView — 50,000 virtualized items (multi-select, icons)");
        section->AddChild(CreateFileListView());
        root->AddChild(std::move(section));
    }

    // ── 3. UITreeView — Folder navigation ─────────────────────────────
    {
        auto section = std::make_unique<Section>(L"3. UITreeView — Nested navigation with lazy loading");
        section->AddChild(CreateFolderTreeView());
        root->AddChild(std::move(section));
    }

    // ── 4. Dialog framework ───────────────────────────────────────────
    {
        auto section = std::make_unique<Section>(L"4. Dialogs — MessageBox, File, Color, Font, Progress");
        section->AddChild(CreateDialogDemo(host));
        root->AddChild(std::move(section));
    }

    return root;
}

} // namespace DragonOS::DragonUI::Demo
