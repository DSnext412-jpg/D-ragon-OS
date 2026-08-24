#include <Explorer2/ExplorerWindow.hpp>
#include <Explorer2/ExplorerDialogs.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/Dialogs/UIMessageBox.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <WindowManager/DragonWindow.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Local helpers
// ============================================================================

namespace {

[[nodiscard]] std::wstring TitleForPath(const std::wstring& path) noexcept
{
    if (path.empty()) { return L"New tab"; }
    if (FileSystem::FileSystemService::IsRootPath(path))
    {
        return path;                                  // "C:\"
    }
    std::wstring name = FileSystem::FileSystemService::GetFileName(path);
    while (!name.empty() && (name.back() == L'\\' || name.back() == L'/'))
    {
        name.pop_back();
    }
    return name.empty() ? path : name;
}

[[nodiscard]] std::wstring LowercaseExtensionOf(const std::wstring& fileName) noexcept
{
    const size_t dot = fileName.rfind(L'.');
    if (dot == std::wstring::npos) { return {}; }
    std::wstring ext = fileName.substr(dot);          // keeps the dot
    CharLowerBuffW(ext.data(), static_cast<DWORD>(ext.size()));
    return ext;
}

} // namespace

// ============================================================================
//  Construction / teardown
// ============================================================================

ExplorerWindow::ExplorerWindow() noexcept
{
    m_root.SetAccessibleName(L"File Explorer");
    m_root.SetAutomationId(L"Explorer2");
}

ExplorerWindow::~ExplorerWindow() noexcept
{
    m_accessibility.Shutdown();
    m_focusManager.UnregisterRoot(&m_root);
}

// ============================================================================
//  Service wiring
// ============================================================================

void ExplorerWindow::SetServices(Services services) noexcept
{
    m_services = services;

    // â”€â”€ Cache-backed services â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_loader = std::make_unique<LoadPipeline>(*m_services.cache);
    m_search = std::make_unique<SearchController>(*m_services.fs, *m_services.cache);
    m_navPane = std::make_unique<NavigationPane>(
        *m_services.fs, *m_services.cache, m_bookmarks);

    // â”€â”€ Remaining panes â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_breadcrumb = std::make_unique<BreadcrumbBar>();
    m_fileList = std::make_unique<FileListPane>();
    m_preview = std::make_unique<PreviewPane>();
    m_status = std::make_unique<StatusBarPane>();

    m_preview->GetControl()->SetVisibility(Visibility::Collapsed);
    m_preview->GetControl()->SetMinSize(240.0f, 0.0f);
    m_navPane->GetControl()->SetMinSize(200.0f, 0.0f);
    m_navPane->GetControl()->SetMaxSize(320.0f, FLT_MAX);

    // ── Assemble the tree ─────────────────────────────────────────────────
    auto topStack = std::make_unique<UIStackPanel>(Orientation::Vertical);
    topStack->AddChild(std::move(m_tabStrip));
    topStack->AddChild(m_ribbon->TakeRoot());
    m_topStack = topStack.get();
    m_root.AddChild(std::move(topStack));
    m_root.SetChildDock(*m_topStack, Dock::Top);

    auto centerRow = std::make_unique<UIDockPanel>();
    m_centerRow = centerRow.get();

    DragonUI::Element* navRaw = m_navPane->GetControl();
    DragonUI::Element* previewRaw = m_preview->GetControl();

    m_centerRow->AddChild(m_navPane->TakeRoot());
    m_centerRow->SetChildDock(*navRaw, Dock::Left);

    m_centerRow->AddChild(m_preview->TakeRoot());
    m_centerRow->SetChildDock(*previewRaw, Dock::Right);

    auto centerColumn = std::make_unique<UIStackPanel>(Orientation::Vertical);
    centerColumn->AddChild(m_breadcrumb->TakeRoot());
    centerColumn->AddChild(m_fileList->TakeRoot());
    m_centerColumn = centerColumn.get();

    m_centerRow->AddChild(std::move(centerColumn));
    m_centerRow->SetChildDock(*m_centerColumn, Dock::Fill);
    m_centerRow->SetLastChildFill(true);

    m_root.AddChild(std::move(centerRow));
    m_root.SetChildDock(*m_centerRow, Dock::Fill);

    DragonUI::Element* statusRaw = m_status->GetControl();
    m_root.AddChild(m_status->TakeRoot());
    m_root.SetChildDock(*statusRaw, Dock::Bottom);
    m_root.SetLastChildFill(false);

    // â”€â”€ Accessibility + focus â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (m_services.theme)
    {
        m_accessibility.Initialize(&m_root, &m_focusManager, m_services.theme);
    }
    m_focusManager.RegisterRoot(&m_root);
    m_focusManager.SetOnFocusChanged(
        [this](Control* focused) { m_accessibility.ReportFocus(focused); });

    WireCallbacks();

    // â”€â”€ First tab â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    OpenTab(m_services.fs->GetKnownFolderPath(FileSystem::KnownFolder::Home));
}

void ExplorerWindow::WireCallbacks() noexcept
{
    // â”€â”€ Tab strip â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    TabStrip::Host tabHost;
    tabHost.onActivate = [this](uint64_t id) {
        for (size_t i = 0; i < m_tabs.size(); ++i)
        {
            if (m_tabs[i].state.id == id) { ActivateTab(i); return; }
        }
    };
    tabHost.onClose = [this](uint64_t id) { CloseTab(id); };
    tabHost.onReorder = [this](size_t from, size_t to) {
        if (from >= m_tabs.size()) { return; }
        Tab tab = std::move(m_tabs[from]);
        m_tabs.erase(m_tabs.begin() + static_cast<ptrdiff_t>(from));
        to = std::min(to, m_tabs.size());
        m_tabs.insert(m_tabs.begin() + static_cast<ptrdiff_t>(to), std::move(tab));
        m_activeIndex = to;
        m_tabStrip->Rebuild();
    };
    tabHost.onNewTabRequest = [this]() {
        OpenTab(m_services.fs->GetKnownFolderPath(FileSystem::KnownFolder::Home));
    };
    tabHost.getTabs = [this]() -> const std::vector<TabState>& {
        static thread_local std::vector<TabState> snapshot;
        snapshot.clear();
        snapshot.reserve(m_tabs.size());
        for (const Tab& t : m_tabs) { snapshot.push_back(t.state); }
        return snapshot;
    };
    tabHost.getActiveTabId = [this]() { return GetActiveTabId(); };
    m_tabStrip->SetHost(std::move(tabHost));

    // â”€â”€ Navigation pane â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_navPane->SetOnNavigate(
        [this](const std::wstring& path) { NavigateActiveTab(path); });
    m_navPane->SetOnPinRequested([this]() {
        if (const Tab* t = ActiveTab())
        {
            if (!m_bookmarks.Contains(t->state.path))
            {
                Bookmark bookmark;
                bookmark.name = TitleForPath(t->state.path);
                bookmark.path = t->state.path;
                bookmark.glyph = IconLibrary::Star;
                m_bookmarks.Add(std::move(bookmark));
                m_navPane->RefreshQuickAccess();
                Announce(L"Pinned to Quick access");
            }
        }
    });

    // â”€â”€ Breadcrumb â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_breadcrumb->SetOnNavigate(
        [this](const std::wstring& path) { NavigateActiveTab(path); });
    m_breadcrumb->SetMenuSink(
        [this](std::unique_ptr<UIContextMenu> menu, float x, float y) {
            PushMenu(std::move(menu), x, y);
        });
    m_breadcrumb->SetHistoryProvider([this]() {
        std::vector<std::wstring> recent;
        if (const Tab* t = ActiveTab())
        {
            recent.reserve(t->history.BackStack().size()
                           + t->history.ForwardStack().size());
            for (auto it = t->history.BackStack().rbegin();
                 it != t->history.BackStack().rend(); ++it)
            {
                recent.push_back(*it);
            }
            for (auto it = t->history.ForwardStack().rbegin();
                 it != t->history.ForwardStack().rend(); ++it)
            {
                recent.push_back(*it);
            }
        }
        return recent;
    });

    // â”€â”€ File list â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_fileList->SetOnActivateEntry(
        [this](const FileSystem::FileEntry& entry) { ActivateEntry(entry); });
    m_fileList->SetOnSelectionChanged([this]() {
        SyncRibbonState();
        UpdateStatusLine();
        UpdatePreviewFromSelection();
        m_accessibility.ReportSelectionChanged(&m_fileList->GetList());
    });
    m_fileList->SetOnSortChanged([this]() {
        SyncRibbonState();
        m_ribbon->RefreshMenus();
    });

    // â”€â”€ Ribbon actions â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    RibbonBar::Actions a;
    a.onBack       = [this]() { NavigateBack(); };
    a.onForward    = [this]() { NavigateForward(); };
    a.onUp         = [this]() { NavigateUp(); };
    a.onRefresh    = [this]() { RefreshView(); };
    a.onNewFolder  = [this]() { BeginNewFolder(); };
    a.onCut        = [this]() { CopyOrCutSelection(true); };
    a.onCopy       = [this]() { CopyOrCutSelection(false); };
    a.onPaste      = [this]() { PasteIntoCurrentFolder(); };
    a.onRename     = [this]() {
        if (const auto* e = m_fileList->GetSingleSelectedEntry()) { BeginRename(*e); }
    };
    a.onDelete     = [this]() { BeginDeleteWithConfirm(); };
    a.onProperties = [this]() {
        const auto selected = m_fileList->GetSelectedEntries();
        if (!selected.empty()) { ShowPropertiesFor(*selected.front()); }
    };
    a.onViewMode      = [this](ViewMode mode) { SetViewMode(mode); };
    a.onSortBy        = [this](SortKey key) { SetSortBy(key); };
    a.onGroupBy       = [this](GroupMode mode) { SetGroupBy(mode); };
    a.onTogglePreview = [this]() { TogglePreviewPane(); };

    a.canGoBack        = [this]() { const Tab* t = ActiveTab(); return t && t->history.CanGoBack(); };
    a.canGoForward     = [this]() { const Tab* t = ActiveTab(); return t && t->history.CanGoForward(); };
    a.currentViewMode  = [this]() { const Tab* t = ActiveTab(); return t ? t->vm->GetViewMode() : ViewMode::Details; };
    a.currentSortKey   = [this]() { const Tab* t = ActiveTab(); return t ? t->vm->GetSortKey() : SortKey::Name; };
    a.currentSortDir   = [this]() { const Tab* t = ActiveTab(); return t ? t->vm->GetSortDir() : SortDir::Ascending; };
    a.currentGroupMode = [this]() { const Tab* t = ActiveTab(); return t ? t->vm->GetGroupMode() : GroupMode::None; };
    a.previewVisible   = [this]() { return m_preview && m_preview->GetControl()->IsVisible(); };
    a.hasSelection     = [this]() { return !m_fileList->GetSelectedEntries().empty(); };
    a.clipboardHasContent = [this]() { return !m_clipboard.empty(); };

    m_ribbon->SetActions(std::move(a));
    m_ribbon->BindSearch(
        [this](const std::wstring& text) { OnSearchTextChanged(text); },
        [this]() { m_focusManager.SetFocus(m_ribbon->GetSearchBox()); });

    // â”€â”€ Search controller â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_search->SetResultsChanged([this]() {
        if (Tab* t = ActiveTab())
        {
            std::vector<FileSystem::FileEntry> matches = m_search->GetMatches();
            t->vm->SetEntries(std::move(matches));
            UpdateStatusLine();
        }
    });
    m_search->SetStatusChanged([this](const std::wstring& text) {
        OnSearchStatus(text);
    });

    // â”€â”€ Context menus â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_contextMenus.SetSink(
        [this](std::unique_ptr<UIContextMenu> menu, float x, float y) {
            PushMenu(std::move(menu), x, y);
        });
    m_contextMenus.SetOpenWithProvider([this](const FileSystem::FileEntry& entry) {
        std::vector<ContextMenuService::OpenWithAction> actions;
        const std::wstring ext = LowercaseExtensionOf(entry.name);
        if (ext.empty()) { return actions; }
        for (const OpenWithEntry& handler : m_openWithHandlers)
        {
            if (handler.extension == ext)
            {
                actions.push_back({ handler.label, handler.action });
            }
        }
        return actions;
    });
}

// ============================================================================
//  Layout / frame
// ============================================================================

void ExplorerWindow::Layout(
    float clientX, float clientY, float clientW, float clientH,
    float viewportW, float viewportH) noexcept
{
    m_clientX = clientX; m_clientY = clientY;
    m_clientW = clientW; m_clientH = clientH;
    m_viewportW = viewportW; m_viewportH = viewportH;

    m_root.Measure(LayoutSlot{ 0.0f, 0.0f, clientW, clientH });
    m_root.Arrange(LayoutSlot{ clientX, clientY, clientW, clientH });

    m_dialogManager.SetViewport(viewportW, viewportH);
}

void ExplorerWindow::Update(float deltaTime) noexcept
{
    if (m_services.fs && m_loader)
    {
        m_loader->Update(*m_services.fs);
    }
    if (m_search)
    {
        m_search->Update();
    }

    if (const Tab* t = ActiveTab())
    {
        m_status->SetDiskFree(StatusBarPane::QueryDiskFreeBytes(t->state.path));
    }

    m_dialogManager.Update(deltaTime, m_focusManager);
}

void ExplorerWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_services.theme)
    {
        return;
    }

    RenderContext ctx(renderer, *m_services.theme);
    m_root.Render(ctx);

    for (const auto& menu : m_openMenus)
    {
        if (menu && menu->IsOpen())
        {
            menu->Render(ctx);
        }
    }

    if (!m_dialogManager.IsEmpty())
    {
        m_dialogManager.Render(ctx, m_viewportW, m_viewportH);
    }
}

// ============================================================================
//  Input pump
// ============================================================================

namespace {
[[nodiscard]] Control* ControlAncestor(Element* element) noexcept
{
    while (element)
    {
        if (auto* control = dynamic_cast<Control*>(element))
        {
            return control;
        }
        element = element->GetParent();
    }
    return nullptr;
}
} // namespace

bool ExplorerWindow::RouteToMenus(const EventArgs& args) noexcept
{
    if (m_openMenus.empty()) { return false; }

    UIContextMenu* top = m_openMenus.back().get();

    if (args.type == EventType::MouseMove || args.type == EventType::MouseDown
        || args.type == EventType::MouseUp)
    {
        const bool inside =
            args.mouse.x >= top->GetX() && args.mouse.x < top->GetX() + top->GetWidth()
            && args.mouse.y >= top->GetY() && args.mouse.y < top->GetY() + top->GetHeight();
        if (!inside && args.type == EventType::MouseDown)
        {
            CloseTopMenu();
            return true;                       // click-away dismisses.
        }
        (void)top->OnMouseEvent(args.type, args.mouse);
        return true;
    }

    if (args.type == EventType::KeyDown)
    {
        (void)top->OnKeyEvent(EventType::KeyDown, args.key);
        return true;
    }
    if (args.type == EventType::TextInput)
    {
        (void)top->OnKeyEvent(EventType::TextInput, args.key);
        return true;
    }
    return false;
}

void ExplorerWindow::HandleMouseMove(float x, float y) noexcept
{
    if (m_dialogManager.HandleMouseMove(x, y)) { return; }

    EventArgs args = EventArgs::MakeMouse(EventType::MouseMove, x, y);
    if (RouteToMenus(args)) { return; }

    Element* hit = m_root.HitTest(x, y);
    Control* target = ControlAncestor(hit);
    if (target)
    {
        (void)target->OnEvent(args);
    }
}

void ExplorerWindow::HandleMouseDown(float x, float y, Input::MouseButton button) noexcept
{
    if (m_dialogManager.HandleMouseDown(x, y, button)) { return; }

    EventArgs downArgs = EventArgs::MakeMouse(EventType::MouseDown, x, y, button);
    if (RouteToMenus(downArgs)) { return; }

    // â”€â”€ Right click â†’ context menus â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (button == Input::MouseButton::Right)
    {
        Element* hit = m_root.HitTest(x, y);
        Control* target = ControlAncestor(hit);
        if (!target) { return; }

        const bool overList =
            m_fileList && &m_fileList->GetList() == target;
        if (overList)
        {
            const FileSystem::FileEntry* underCursor = m_fileList->GetEntryAtPoint(x, y);
            if (underCursor)
            {
                ContextMenuService::State state = BuildMenuState(false);
                state.selection.push_back(*underCursor);
                m_contextMenus.ShowItemMenu(x, y, state);
            }
            else
            {
                m_contextMenus.ShowBackgroundMenu(x, y, BuildMenuState(false));
            }
        }
        return;
    }

    if (button != Input::MouseButton::Left) { return; }

    // â”€â”€ Group-header toggle on single click (before selection logic) â”€â”€â”€â”€
    Element* hit = m_root.HitTest(x, y);
    Control* target = ControlAncestor(hit);

    const bool overList = m_fileList && target && &m_fileList->GetList() == target;

    if (overList && !(m_services.mouse && m_services.mouse->WasDoubleClicked(button)))
    {
        if (const FileRow* row = m_fileList->GetRowAtPoint(x, y); row && row->IsHeader())
        {
            if (Tab* t = ActiveTab())
            {
                t->vm->ToggleGroupCollapsed(row->groupLabel);
            }
            return;
        }
    }

    // â”€â”€ Details column-header interception (sorting lives in the VM) â”€â”€â”€â”€
    if (overList && m_fileList->HandleHeaderClick(x, y))
    {
        return;
    }

    // â”€â”€ Standard press routing (mirrors WindowHost) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_pressedControl = target;
    if (target)
    {
        target->SetControlState(ControlState::Pressed);
        if (target->IsFocusable())
        {
            m_focusManager.SetFocus(target);
        }
        downArgs.mouse.clickCount =
            (m_services.mouse && m_services.mouse->WasDoubleClicked(button)) ? 2 : 1;
        (void)target->OnEvent(downArgs);

        // Double-click activation on entries.
        if (overList && downArgs.mouse.clickCount == 2)
        {
            if (const FileSystem::FileEntry* entry = m_fileList->GetEntryAtPoint(x, y))
            {
                ActivateEntry(*entry);
            }
        }
    }
}

void ExplorerWindow::HandleMouseUp(float x, float y, Input::MouseButton button) noexcept
{
    if (m_dialogManager.HandleMouseUp(x, y, button)) { return; }

    EventArgs upArgs = EventArgs::MakeMouse(EventType::MouseUp, x, y, button);
    if (RouteToMenus(upArgs)) { return; }

    Control* releaseTarget = m_pressedControl;
    m_pressedControl = nullptr;
    if (!releaseTarget) { return; }

    releaseTarget->SetControlState(
        ControlAncestor(m_root.HitTest(x, y)) == releaseTarget
            ? ControlState::Hover : ControlState::Normal);

    (void)releaseTarget->OnEvent(upArgs);

    if (ControlAncestor(m_root.HitTest(x, y)) == releaseTarget)
    {
        EventArgs clickArgs = EventArgs::MakeMouse(EventType::Click, x, y, button);
        (void)releaseTarget->OnEvent(clickArgs);
    }
}

void ExplorerWindow::HandleMouseWheel(float delta, float x, float y) noexcept
{
    if (m_dialogManager.HandleMouseWheel(delta, x, y)) { return; }

    // Wheel is delivered as a MouseMove event carrying wheelDelta (the same
    // convention WindowHost uses).
    EventArgs args = EventArgs::MakeMouse(EventType::MouseMove, x, y);
    args.mouse.wheelDelta = delta;
    if (RouteToMenus(args)) { return; }

    Control* target = ControlAncestor(m_root.HitTest(x, y));
    if (target)
    {
        (void)target->OnEvent(args);
    }
}

void ExplorerWindow::HandleKey(
    Input::KeyCode key, bool ctrl, bool shift, bool alt, bool isRepeat) noexcept
{
    // 1. Dialogs first (modal).
    if (m_dialogManager.HandleKey(key, ctrl, shift, alt)) { return; }

    KeyEventArgs keyArgs;
    keyArgs.key = key;
    keyArgs.isRepeat = isRepeat;
    keyArgs.ctrl = ctrl;
    keyArgs.shift = shift;
    keyArgs.alt = alt;

    // 2. Open menus consume navigation keys.
    if (!m_openMenus.empty())
    {
        if (key == Input::KeyCode::Escape)
        {
            CloseTopMenu();
            return;
        }
        (void)m_openMenus.back()->OnKeyEvent(EventType::KeyDown, keyArgs);
        return;
    }

    // 3. Breadcrumb edit mode: Escape leaves it.
    if (key == Input::KeyCode::Escape && m_breadcrumb->IsEditing())
    {
        m_breadcrumb->CancelEditing();
        m_focusManager.SetFocus(&m_fileList->GetList());
        return;
    }

    // 4. Focused control gets the key first (text carets, tree nav...).
    EventArgs evt;
    evt.type = EventType::KeyDown;
    evt.key = keyArgs;

    Control* focused = m_focusManager.GetFocused();
    if (focused && focused->OnEvent(evt))
    {
        return;
    }

    // 5. Application shortcuts.
    if (HandleShortcut(key, ctrl, shift, alt))
    {
        return;
    }

    // 6. Logical focus navigation fallback.
    switch (key)
    {
    case Input::KeyCode::Up:    m_focusManager.MoveFocusDirection(FocusDirection::Up);    break;
    case Input::KeyCode::Down:  m_focusManager.MoveFocusDirection(FocusDirection::Down);  break;
    case Input::KeyCode::Left:  m_focusManager.MoveFocusDirection(FocusDirection::Left);  break;
    case Input::KeyCode::Right: m_focusManager.MoveFocusDirection(FocusDirection::Right); break;
    case Input::KeyCode::Return:m_focusManager.ActivateFocused();                        break;
    default: break;
    }
}

void ExplorerWindow::HandleCharacter(wchar_t ch) noexcept
{
    if (m_dialogManager.HandleText(ch)) { return; }

    if (!m_openMenus.empty())
    {
        KeyEventArgs args;
        args.character = ch;
        (void)m_openMenus.back()->OnKeyEvent(EventType::TextInput, args);
        return;
    }

    if (Control* focused = m_focusManager.GetFocused())
    {
        EventArgs evt;
        evt.type = EventType::TextInput;
        evt.key = { Input::KeyCode::Unknown, ch };
        (void)focused->OnEvent(evt);
    }
}

// ============================================================================
//  Shortcuts
// ============================================================================

bool ExplorerWindow::HandleShortcut(
    Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept
{
    using IK = Input::KeyCode;

    // Clipboard / editing.
    if (ctrl && !shift && !alt)
    {
        switch (key)
        {
        case IK::C: CopyOrCutSelection(false); return true;
        case IK::X: CopyOrCutSelection(true);  return true;
        case IK::V: PasteIntoCurrentFolder();  return true;
        case IK::A: SelectAll();               return true;
        case IK::F:                             // focus search box
            m_focusManager.SetFocus(m_ribbon->GetSearchBox());
            return true;
        case IK::L:
        case IK::F6:
            m_breadcrumb->BeginEditing();
            m_focusManager.SetFocus(m_breadcrumb->GetEditBox());
            return true;
        case IK::T: OpenTab(ActiveTab() ? ActiveTab()->state.path
                                        : m_services.fs->GetKnownFolderPath(FileSystem::KnownFolder::Home));
                    return true;
        case IK::W: if (const Tab* t = ActiveTab()) { CloseTab(t->state.id); } return true;
        case IK::D1: SetViewMode(ViewMode::Details); return true;
        case IK::D2: SetViewMode(ViewMode::List);    return true;
        case IK::D3: SetViewMode(ViewMode::Tiles);   return true;
        default: break;
        }
    }

    if (ctrl && shift && key == IK::T)
    {
        RestoreClosedTab();
        return true;
    }

    if (!ctrl && !alt)
    {
        switch (key)
        {
        case IK::Delete: BeginDeleteWithConfirm(); return true;
        case IK::F2:
            if (const auto* e = m_fileList->GetSingleSelectedEntry()) { BeginRename(*e); }
            return true;
        case IK::F5: RefreshView(); return true;
        case IK::Back:
            if (!shift) { NavigateUp(); }
            return true;
        case IK::Return:
            if (const auto* e = m_fileList->GetSingleSelectedEntry())
            {
                ActivateEntry(*e);
                return true;
            }
            break;
        default: break;
        }
    }

    if (alt && !ctrl)
    {
        if (key == IK::Left)  { NavigateBack();    return true; }
        if (key == IK::Right) { NavigateForward(); return true; }
        if (key == IK::Up)    { NavigateUp();      return true; }
    }

    return false;
}

// ============================================================================
//  Tabs
// ============================================================================

ExplorerWindow::Tab* ExplorerWindow::ActiveTab() noexcept
{
    return m_activeIndex < m_tabs.size() ? &m_tabs[m_activeIndex] : nullptr;
}

const ExplorerWindow::Tab* ExplorerWindow::ActiveTab() const noexcept
{
    return m_activeIndex < m_tabs.size() ? &m_tabs[m_activeIndex] : nullptr;
}

void ExplorerWindow::OpenTab(
    const std::wstring& path, ViewMode viewMode, bool activate) noexcept
{
    Tab tab;
    tab.state.id = m_nextTabId++;
    tab.state.viewMode = viewMode;
    tab.vm = std::make_unique<ExplorerViewModel>();
    tab.vm->SetCurrentPath(path);
    tab.history.Reset(path);

    m_tabs.push_back(std::move(tab));

    if (activate)
    {
        ActivateTab(m_tabs.size() - 1);
    }
    else
    {
        NavigateInTab(m_tabs.back(), path, /*pushHistory=*/false);
        m_tabStrip->Rebuild();
    }
}

void ExplorerWindow::ActivateTab(size_t index) noexcept
{
    if (index >= m_tabs.size()) { return; }
    m_activeIndex = index;
    RebindContentToActiveTab();
    m_tabStrip->Rebuild();
}

void ExplorerWindow::CloseTab(uint64_t tabId) noexcept
{
    for (size_t i = 0; i < m_tabs.size(); ++i)
    {
        if (m_tabs[i].state.id != tabId) { continue; }

        ClosedTab closed;
        closed.path = m_tabs[i].state.path;
        closed.viewMode = m_tabs[i].state.viewMode;
        m_closedTabs.push_back(std::move(closed));
        if (m_closedTabs.size() > 16)
        {
            m_closedTabs.erase(m_closedTabs.begin());
        }

        m_tabs.erase(m_tabs.begin() + static_cast<ptrdiff_t>(i));

        if (m_tabs.empty())
        {
            // Keep one tab alive at all times.
            OpenTab(m_services.fs->GetKnownFolderPath(FileSystem::KnownFolder::Home));
            return;
        }

        if (m_activeIndex >= m_tabs.size() || i == m_activeIndex)
        {
            ActivateTab(std::min(i, m_tabs.size() - 1));
        }
        else
        {
            m_tabStrip->Rebuild();
        }
        return;
    }
}

void ExplorerWindow::RestoreClosedTab() noexcept
{
    if (m_closedTabs.empty()) { return; }
    ClosedTab closed = std::move(m_closedTabs.back());
    m_closedTabs.pop_back();
    OpenTab(closed.path, closed.viewMode, true);
    Announce(L"Reopened tab " + TitleForPath(closed.path));
}

void ExplorerWindow::RebindContentToActiveTab() noexcept
{
    Tab* t = ActiveTab();
    if (!t) { return; }

    m_fileList->BindModel(*t->vm);
    m_fileList->ApplyViewMode(t->state.viewMode);
    m_fileList->ClearSelection();
    m_fileList->NotifySortChanged();

    m_breadcrumb->SetPath(t->state.path);
    m_breadcrumb->CancelEditing();
    m_navPane->HighlightPath(t->state.path);

    m_ribbon->SetSearchText(L"");
    m_ribbon->SyncState();
    m_search->CancelDeepSearch();
    m_preview->Clear();

    UpdateStatusLine();

    // Kick a cache-first load for the tab's current folder.
    RequestListing(t->state.path);
}

void ExplorerWindow::SyncRibbonState() noexcept
{
    if (m_ribbon)
    {
        m_ribbon->SyncState();
    }
}

// ============================================================================
//  Navigation / data flow
// ============================================================================

void ExplorerWindow::NavigateInTab(Tab& tab, const std::wstring& path,
                                   bool pushHistory) noexcept
{
    if (path.empty()) { return; }
    if (!m_services.fs->Exists(path))
    {
        m_status->SetOperation(L"Location not found: " + path, -1.0f);
        m_accessibility.Alert(L"Location not found");
        return;
    }

    if (pushHistory)
    {
        tab.history.Push(path);
    }

    tab.state.path = path;
    tab.state.title = TitleForPath(path);
    tab.vm->SetCurrentPath(path);

    // Leaving search mode on navigation.
    m_search->CancelDeepSearch();
    m_search->SetQuery(path, L"");           // clears matches/status
    tab.vm->SetQuickFilter(L"");
    tab.vm->SetSearchResultsMode(false);
    tab.vm->ExpandAllGroups();
    m_ribbon->SetSearchText(L"");

    m_breadcrumb->SetPath(path);
    m_navPane->HighlightPath(path);
    m_tabStrip->Rebuild();
    SyncRibbonState();

    m_status->SetOperation(L"Loading " + path + L"\x2026", -2.0f);
    RequestListing(path);
}

void ExplorerWindow::RequestListing(const std::wstring& path) noexcept
{
    m_loader->Request(path, [this](const FileSystem::DirectoryResult& result) {
        OnEntriesLoaded(result);
    });
}

void ExplorerWindow::NavigateActiveTab(const std::wstring& path, bool pushHistory) noexcept
{
    if (Tab* t = ActiveTab())
    {
        NavigateInTab(*t, path, pushHistory);
    }
}

void ExplorerWindow::NavigateBack() noexcept
{
    if (Tab* t = ActiveTab())
    {
        if (const std::wstring path = t->history.GoBack(); !path.empty())
        {
            NavigateInTab(*t, path, /*pushHistory=*/false);
        }
    }
}

void ExplorerWindow::NavigateForward() noexcept
{
    if (Tab* t = ActiveTab())
    {
        if (const std::wstring path = t->history.GoForward(); !path.empty())
        {
            NavigateInTab(*t, path, /*pushHistory=*/false);
        }
    }
}

void ExplorerWindow::NavigateUp() noexcept
{
    if (Tab* t = ActiveTab())
    {
        const std::wstring parent =
            FileSystem::FileSystemService::GetParentPath(t->state.path);
        if (!parent.empty() && parent != t->state.path)
        {
            NavigateInTab(*t, parent, /*pushHistory=*/true);
        }
    }
}

void ExplorerWindow::OnEntriesLoaded(const FileSystem::DirectoryResult& result) noexcept
{
    Tab* t = ActiveTab();
    if (!t) { return; }

    if (!result.success)
    {
        m_status->SetOperation(result.errorMessage.empty()
            ? L"Could not read this folder." : result.errorMessage, -1.0f);
        m_accessibility.Alert(L"Folder could not be read");
        return;
    }

    std::vector<FileSystem::FileEntry> entries = result.entries;
    t->vm->SetEntries(std::move(entries));

    m_status->ClearOperation();
    UpdateStatusLine();

    wchar_t count[64];
    (void)swprintf_s(count, L"%zu items", t->vm->GetEntryCount());
    Announce(count);
}

void ExplorerWindow::UpdateStatusLine() noexcept
{
    const Tab* t = ActiveTab();
    if (!t) { return; }

    m_status->SetItemSummary(
        t->vm->GetEntryCount(),
        m_fileList->GetSelectedEntries().size(),
        t->vm->GetHiddenByFilterCount());
    m_status->SetDiskFree(StatusBarPane::QueryDiskFreeBytes(t->state.path));
}

// ============================================================================
//  Search
// ============================================================================

void ExplorerWindow::OnSearchTextChanged(const std::wstring& text) noexcept
{
    Tab* t = ActiveTab();
    if (!t) { return; }

    if (text.empty())
    {
        m_search->CancelDeepSearch();
        m_search->SetQuery(t->state.path, L"");
        t->vm->SetSearchResultsMode(false);
        t->vm->SetQuickFilter(L"");
        RequestListing(t->state.path);       // restore normal listing
        m_status->ClearOperation();
        return;
    }

    t->vm->SetSearchResultsMode(true);
    t->vm->SetQuickFilter(L"");
    m_status->SetOperation(L"Searching\x2026", -2.0f);
    m_search->SetQuery(t->state.path, text);
}

void ExplorerWindow::OnSearchStatus(const std::wstring& text) noexcept
{
    if (m_search->HasQuery())
    {
        m_status->SetOperation(text, -2.0f);
    }
}

// ============================================================================
//  Preview
// ============================================================================

void ExplorerWindow::UpdatePreviewFromSelection() noexcept
{
    if (!m_preview || !m_preview->GetControl()->IsVisible()) { return; }

    const auto selected = m_fileList->GetSelectedEntries();
    if (selected.empty())
    {
        m_preview->Clear();
    }
    else if (selected.size() == 1)
    {
        m_preview->ShowEntry(*selected.front());
    }
    else
    {
        m_preview->ShowMultiSelection(selected.size());
    }
}

void ExplorerWindow::TogglePreviewPane() noexcept
{
    if (!m_preview) { return; }
    DragonUI::Element* previewControl = m_preview->GetControl();
    previewControl->SetVisibility(
        previewControl->IsVisible() ? Visibility::Collapsed : Visibility::Visible);
    m_root.Invalidate();
    UpdatePreviewFromSelection();
    SyncRibbonState();
}

// ============================================================================
//  Context-menu plumbing
// ============================================================================

ContextMenuService::State ExplorerWindow::BuildMenuState(bool includeSelection) const noexcept
{
    ContextMenuService::State state;
    state.target = const_cast<ExplorerWindow*>(this);
    if (const Tab* t = ActiveTab())
    {
        state.currentPath = t->state.path;
        state.viewMode = t->vm->GetViewMode();
        state.sortKey = t->vm->GetSortKey();
        state.groupMode = t->vm->GetGroupMode();
    }
    state.clipboardHasContent = !m_clipboard.empty();

    if (includeSelection)
    {
        for (const FileSystem::FileEntry* entry : m_fileList->GetSelectedEntries())
        {
            state.selection.push_back(*entry);
        }
    }
    return state;
}

void ExplorerWindow::PushMenu(
    std::unique_ptr<UIContextMenu> menu, float x, float y) noexcept
{
    if (!menu) { return; }
    menu->ShowAt(x, y);
    m_openMenus.push_back(std::move(menu));
}

void ExplorerWindow::CloseTopMenu() noexcept
{
    if (m_openMenus.empty()) { return; }
    m_openMenus.back()->Close();
    m_openMenus.pop_back();
}

// ============================================================================
//  File operations (IContextMenuTarget)
// ============================================================================

void ExplorerWindow::OpenEntry(const FileSystem::FileEntry& entry)
{
    ActivateEntry(entry);
}

void ExplorerWindow::ActivateEntry(const FileSystem::FileEntry& entry) noexcept
{
    if (entry.IsDirectory())
    {
        NavigateActiveTab(entry.fullPath);
        return;
    }

    // Plugin-registered handlers win; otherwise surface guidance.
    const std::wstring ext = LowercaseExtensionOf(entry.name);
    for (const OpenWithEntry& handler : m_openWithHandlers)
    {
        if (handler.extension == ext)
        {
            handler.action(entry);
            return;
        }
    }

    m_accessibility.Announce(L"No application registered for " + entry.name);
    m_status->SetOperation(L"No app associated with " + ext, -1.0f);
}

void ExplorerWindow::CutSelection()      { CopyOrCutSelection(true); }
void ExplorerWindow::CopySelection()     { CopyOrCutSelection(false); }

void ExplorerWindow::CopyOrCutSelection(bool cut) noexcept
{
    m_clipboard.clear();
    m_clipboardIsCut = cut;
    for (const FileSystem::FileEntry* entry : m_fileList->GetSelectedEntries())
    {
        ClipboardEntry clip;
        clip.sourcePath = entry->fullPath;
        clip.isDirectory = entry->IsDirectory();
        m_clipboard.push_back(std::move(clip));
    }

    Announce(cut ? L"Cut" : L"Copied");
    SyncRibbonState();
}

std::wstring ExplorerWindow::UniqueDestinationName(
    const std::wstring& directory, const std::wstring& fileName) const noexcept
{
    if (!m_services.fs->Exists(FileSystem::FileSystemService::Combine(directory, fileName)))
    {
        return fileName;
    }

    const size_t dot = fileName.rfind(L'.');
    const std::wstring stem =
        (dot > 0) ? fileName.substr(0, dot) : fileName;
    const std::wstring extension =
        (dot > 0) ? fileName.substr(dot) : std::wstring{};

    for (unsigned counter = 2; counter < 4096; ++counter)
    {
        wchar_t suffix[24];
        (void)swprintf_s(suffix, L" (%u)", counter);
        const std::wstring candidate = directory.empty()
            ? stem + suffix + extension
            : FileSystem::FileSystemService::Combine(directory, stem + suffix + extension);
        if (!m_services.fs->Exists(candidate))
        {
            return stem + suffix + extension;
        }
    }
    return fileName;
}

void ExplorerWindow::PasteIntoCurrentFolder()
{
    const Tab* t = ActiveTab();
    if (!t || m_clipboard.empty()) { return; }

    unsigned moved = 0;
    unsigned failed = 0;
    for (const ClipboardEntry& clip : m_clipboard)
    {
        const std::wstring fileName =
            FileSystem::FileSystemService::GetFileName(clip.sourcePath);
        std::wstring destination = FileSystem::FileSystemService::Combine(
            t->state.path,
            m_clipboardIsCut ? fileName : UniqueDestinationName(t->state.path, fileName));

        const bool ok = m_clipboardIsCut
            ? m_services.fs->MoveItem(clip.sourcePath, destination)
            : m_services.fs->CopyItem(clip.sourcePath, destination, false);
        ok ? ++moved : ++failed;
    }

    if (m_clipboardIsCut)
    {
        m_clipboard.clear();
    }

    m_services.cache->Invalidate(t->state.path);
    RefreshView();
    UpdateStatusLine();

    wchar_t message[96];
    (void)swprintf_s(message, L"%u item(s) %s, %u failed",
                     moved, m_clipboardIsCut ? L"moved" : L"copied", failed);
    Announce(message);
}

void ExplorerWindow::DeleteSelection()   { BeginDeleteWithConfirm(); }

void ExplorerWindow::BeginDeleteWithConfirm() noexcept
{
    const auto selected = m_fileList->GetSelectedEntries();
    if (selected.empty()) { return; }

    wchar_t prompt[160];
    (void)swprintf_s(prompt, L"Delete %zu item(s) permanently?",
                     selected.size());

    auto box = UIMessageBox::Create(
        L"Delete", prompt, MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    box->SetOnClosed([this](UIDialog&, DialogResult result) {
        if (result != DialogResult::Yes) { return; }

        unsigned deleted = 0;
        unsigned failed = 0;
        for (const FileSystem::FileEntry& entry : m_deleteTargets)
        {
            const bool ok = entry.IsDirectory()
                ? m_services.fs->EraseDirectory(entry.fullPath, true)
                : m_services.fs->EraseFile(entry.fullPath);
            ok ? ++deleted : ++failed;
        }
        m_deleteTargets.clear();

        if (const Tab* t = ActiveTab())
        {
            m_services.cache->Invalidate(t->state.path);
        }
        RefreshView();

        wchar_t message[96];
        (void)swprintf_s(message, L"%u deleted, %u failed", deleted, failed);
        Announce(message);
    });

    m_deleteTargets.clear();
    m_deleteTargets.reserve(selected.size());
    for (const FileSystem::FileEntry* entry : selected)
    {
        m_deleteTargets.push_back(*entry);
    }

    ShowDialog(std::move(box));
}

void ExplorerWindow::RenameSelection()
{
    if (const auto* entry = m_fileList->GetSingleSelectedEntry())
    {
        BeginRename(*entry);
    }
}

void ExplorerWindow::BeginRename(const FileSystem::FileEntry& entry) noexcept
{
    auto dialog = ExplorerDialogs::CreateRename(entry.name,
        [this, source = entry.fullPath](const std::wstring& newName) {
            const std::wstring parent =
                FileSystem::FileSystemService::GetParentPath(source);
            const std::wstring target =
                FileSystem::FileSystemService::Combine(parent, newName);

            if (m_services.fs->RenameItem(source, target))
            {
                m_services.cache->Invalidate(parent);
                RefreshView();
                m_fileList->RevealEntry(target);
                Announce(L"Renamed to " + newName);
            }
            else
            {
                m_accessibility.Alert(L"Rename failed");
                m_status->SetOperation(L"Rename failed", -1.0f);
            }
        });
    ShowDialog(std::move(dialog));
}

void ExplorerWindow::NewFolder()         { BeginNewFolder(); }

void ExplorerWindow::BeginNewFolder() noexcept
{
    const Tab* t = ActiveTab();
    if (!t) { return; }

    const std::wstring directory = t->state.path;
    auto dialog = ExplorerDialogs::CreateNewFolder(
        [this, directory](const std::wstring& chosen) {
            const std::wstring unique =
                UniqueDestinationName(directory, chosen);
            const std::wstring fullPath =
                FileSystem::FileSystemService::Combine(directory, unique);

            if (m_services.fs->CreateFolder(fullPath))
            {
                m_services.cache->Invalidate(directory);
                RefreshView();
                m_fileList->RevealEntry(fullPath);
                Announce(L"Created folder " + unique);
            }
            else
            {
                m_accessibility.Alert(L"Folder creation failed");
                m_status->SetOperation(L"Could not create folder", -1.0f);
            }
        });
    ShowDialog(std::move(dialog));
}

void ExplorerWindow::RefreshView()
{
    if (const Tab* t = ActiveTab())
    {
        m_services.cache->Invalidate(t->state.path);
        RequestListing(t->state.path);
    }
}

void ExplorerWindow::SelectAll()
{
    m_fileList->SelectAllEntries();
}

void ExplorerWindow::SetViewMode(ViewMode mode)
{
    if (Tab* t = ActiveTab())
    {
        t->state.viewMode = mode;
        t->vm->SetViewMode(mode);
        m_fileList->ApplyViewMode(mode);
    }
    SyncRibbonState();
    m_ribbon->RefreshMenus();
}

void ExplorerWindow::SetSortBy(SortKey key)
{
    if (Tab* t = ActiveTab())
    {
        t->vm->ToggleSort(key);
        m_fileList->NotifySortChanged();
    }
    SyncRibbonState();
    m_ribbon->RefreshMenus();
}

void ExplorerWindow::SetGroupBy(GroupMode mode)
{
    if (Tab* t = ActiveTab())
    {
        t->vm->SetGroupMode(mode);
    }
    m_ribbon->RefreshMenus();
}

void ExplorerWindow::ShowProperties(const FileSystem::FileEntry* entry)
{
    if (entry)
    {
        ShowPropertiesFor(*entry);
        return;
    }
    const auto selected = m_fileList->GetSelectedEntries();
    if (!selected.empty())
    {
        ShowPropertiesFor(*selected.front());
    }
}

void ExplorerWindow::ShowPropertiesFor(const FileSystem::FileEntry& entry) noexcept
{
    ShowDialog(ExplorerDialogs::CreateProperties(entry));
}

// ============================================================================
//  Dialogs
// ============================================================================

void ExplorerWindow::ShowDialog(std::unique_ptr<UIDialog> dialog) noexcept
{
    if (!dialog) { return; }
    ExplorerDialogs::CenterOverClient(
        *dialog, m_clientX, m_clientY, m_clientW, m_clientH,
        m_viewportW, m_viewportH);
    m_accessibility.ReportStructuredChange();
    m_dialogManager.ShowDialog(std::move(dialog), m_focusManager);
}

// ============================================================================
//  Plugin host surface
// ============================================================================

void ExplorerWindow::AddContextMenuExtension(IExplorerContextMenuExtension* extension) noexcept
{
    m_contextMenus.AddExtension(extension);
    Announce(L"Context menu extension registered");
}

void ExplorerWindow::AddPreviewProvider(IPreviewProvider* provider) noexcept
{
    if (m_preview)
    {
        m_preview->RegisterProvider(provider);
    }
}

void ExplorerWindow::AddOpenWithHandler(
    const std::wstring& dotExtension,
    const std::wstring& label,
    std::function<void(const FileSystem::FileEntry&)> action) noexcept
{
    std::wstring ext = dotExtension;
    if (!ext.empty() && ext.front() != L'.')
    {
        ext.insert(ext.begin(), L'.');
    }
    CharLowerBuffW(ext.data(), static_cast<DWORD>(ext.size()));

    OpenWithEntry entry;
    entry.extension = std::move(ext);
    entry.label = label;
    entry.action = std::move(action);
    m_openWithHandlers.push_back(std::move(entry));
}

void ExplorerWindow::Announce(const std::wstring& text) noexcept
{
    m_accessibility.Announce(text);
}

} // namespace DragonOS::Explorer2

