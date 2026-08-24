// ============================================================================
//  ExplorerWindow.hpp — Explorer 2.0 window assembly + input pump.
//
//  Owns the full element tree for one Explorer window:
//
//      ┌──────────────────────────────────────────────┐
//      │ TabStrip                                     │
//      ├──────────────────────────────────────────────┤
//      │ RibbonBar  (nav cluster | actions | search)  │
//      ├────────┬─────────────────────────┬───────────┤
//      │ Nav    │ BreadcrumbBar           │ Preview   │
//      │ Pane   ├─────────────────────────┤ Pane      │
//      │        │ FileListPane            │ (opt.)    │
//      ├────────┴─────────────────────────┴───────────┤
//      │ StatusBarPane                                │
//      └──────────────────────────────────────────────┘
//
//  The window runs its own Measure/Arrange pump each frame (mirroring
//  WindowHost semantics), routes InputEvents through dialogs → menus →
//  content, and implements all file operations plus accessibility wiring.
// ============================================================================

#pragma once

#include <Explorer2/BreadcrumbBar.hpp>
#include <Explorer2/ContextMenuService.hpp>
#include <Explorer2/DirectoryCache.hpp>
#include <Explorer2/ExplorerTypes.hpp>
#include <Explorer2/ExplorerViewModel.hpp>
#include <Explorer2/FileListPane.hpp>
#include <Explorer2/LoadPipeline.hpp>
#include <Explorer2/NavigationPane.hpp>
#include <Explorer2/PreviewPane.hpp>
#include <Explorer2/RibbonBar.hpp>
#include <Explorer2/SearchController.hpp>
#include <Explorer2/StatusBarPane.hpp>
#include <Explorer2/TabStrip.hpp>

#include <DragonUI/Accessibility/AccessibilityManager.hpp>
#include <DragonUI/Controls/DockPanel.hpp>
#include <DragonUI/Core/Event.hpp>
#include <DragonUI/Core/FocusManager.hpp>
#include <DragonUI/Dialogs/DialogManager.hpp>
#include <FileSystem/FileEntry.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Input/InputEvent.hpp>
#include <Input/MouseButtons.hpp>
#include <Theme/ThemeManager.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace DragonOS {

namespace WindowManager { class DragonWindow; }
namespace Graphics { class Renderer; }
namespace Input { class MouseManager; }

namespace Explorer2 {

class ExplorerWindow final : public IContextMenuTarget {
public:
    struct Services final {
        FileSystem::FileSystemService* fs{};
        Theme::ThemeManager* theme{};
        Input::MouseManager* mouse{};
        DirectoryCache* cache{};                 ///< Shared across windows.
        DragonUI::AccessibilityNotificationHub*
            sharedNotifications{ nullptr };      ///< Optional cross-window hub.
    };

    ExplorerWindow() noexcept;
    ~ExplorerWindow() noexcept override;

    ExplorerWindow(const ExplorerWindow&) = delete;
    ExplorerWindow& operator=(const ExplorerWindow&) = delete;

    // ── Hosting ─────────────────────────────────────────────────────────

    void SetWindow(WindowManager::DragonWindow& window) noexcept { m_window = &window; }
    void SetServices(Services services) noexcept;

    /// Recomputes layout from the owning DragonWindow's client rect.
    void Layout(float clientX, float clientY, float clientW, float clientH,
                float viewportW, float viewportH) noexcept;

    void Update(float deltaTime = 0.0f) noexcept;   ///< Loads, search, dialogs.
    void Render(Graphics::Renderer& renderer) noexcept;

    // ── Input routing (called by ExplorerSystem per frame) ──────────────

    void HandleMouseMove(float x, float y) noexcept;
    void HandleMouseDown(float x, float y, Input::MouseButton button) noexcept;
    void HandleMouseUp(float x, float y, Input::MouseButton button) noexcept;
    void HandleMouseWheel(float delta, float x, float y) noexcept;
    void HandleKey(Input::KeyCode key, bool ctrl, bool shift, bool alt,
                   bool isRepeat) noexcept;
    void HandleCharacter(wchar_t ch) noexcept;

    // ── Tabs ────────────────────────────────────────────────────────────

    void OpenTab(const std::wstring& path, ViewMode viewMode = ViewMode::Details,
                 bool activate = true) noexcept;
    void CloseTab(uint64_t tabId) noexcept;
    void RestoreClosedTab() noexcept;

    [[nodiscard]] uint64_t GetActiveTabId() const noexcept
    {
        return m_activeIndex < m_tabs.size() ? m_tabs[m_activeIndex].state.id : 0;
    }

    // ── Plugin host surface (delegated from ExplorerSystem) ─────────────

    void AddContextMenuExtension(IExplorerContextMenuExtension* extension) noexcept;
    void AddPreviewProvider(IPreviewProvider* provider) noexcept;
    void AddOpenWithHandler(const std::wstring& dotExtension,
                            const std::wstring& label,
                            std::function<void(const FileSystem::FileEntry&)> action) noexcept;
    void Announce(const std::wstring& text) noexcept;

    // ── IContextMenuTarget ──────────────────────────────────────────────

    void OpenEntry(const FileSystem::FileEntry& entry) override;
    void CutSelection() override;
    void CopySelection() override;
    void PasteIntoCurrentFolder() override;
    void DeleteSelection() override;
    void RenameSelection() override;
    void ShowProperties(const FileSystem::FileEntry* entry) override;
    void NewFolder() override;
    void RefreshView() override;
    void SelectAll() override;
    void SetViewMode(ViewMode mode) override;
    void SetSortBy(SortKey key) override;
    void SetGroupBy(GroupMode mode) override;

private:
    struct Tab final {
        TabState state;
        std::unique_ptr<ExplorerViewModel> vm;
        NavHistory history;
    };

    // ── Tab plumbing ────────────────────────────────────────────────────

    [[nodiscard]] Tab* ActiveTab() noexcept;
    [[nodiscard]] const Tab* ActiveTab() const noexcept;
    void ActivateTab(size_t index) noexcept;
    void RebindContentToActiveTab() noexcept;
    void SyncRibbonState() noexcept;

    // ── Navigation / data flow ──────────────────────────────────────────

    void NavigateInTab(Tab& tab, const std::wstring& path,
                       bool pushHistory = true) noexcept;
    void RequestListing(const std::wstring& path) noexcept;
    void NavigateActiveTab(const std::wstring& path, bool pushHistory = true) noexcept;
    void NavigateBack() noexcept;
    void NavigateForward() noexcept;
    void NavigateUp() noexcept;
    void OnEntriesLoaded(const FileSystem::DirectoryResult& result) noexcept;
    void UpdateStatusLine() noexcept;

    // ── Search ──────────────────────────────────────────────────────────

    void OnSearchTextChanged(const std::wstring& text) noexcept;
    void OnSearchStatus(const std::wstring& text) noexcept;

    // ── Preview ─────────────────────────────────────────────────────────

    void UpdatePreviewFromSelection() noexcept;
    void TogglePreviewPane() noexcept;

    // ── File operations ─────────────────────────────────────────────────

    void ActivateEntry(const FileSystem::FileEntry& entry) noexcept;
    void CopyOrCutSelection(bool cut) noexcept;
    [[nodiscard]] std::wstring UniqueDestinationName(
        const std::wstring& directory, const std::wstring& fileName) const noexcept;
    void BeginDeleteWithConfirm() noexcept;
    void BeginRename(const FileSystem::FileEntry& entry) noexcept;
    void BeginNewFolder() noexcept;
    void ShowPropertiesFor(const FileSystem::FileEntry& entry) noexcept;
    void ShowDialog(std::unique_ptr<DragonUI::UIDialog> dialog) noexcept;
    void CloseTopMenu() noexcept;

    // ── Context menu plumbing ───────────────────────────────────────────

    [[nodiscard]] ContextMenuService::State BuildMenuState(
        bool includeSelection) const noexcept;
    void PushMenu(std::unique_ptr<DragonUI::UIContextMenu> menu,
                  float x, float y) noexcept;
    [[nodiscard]] bool RouteToMenus(const DragonUI::EventArgs& args) noexcept;

    // ── Keyboard shortcuts ──────────────────────────────────────────────

    [[nodiscard]] bool HandleShortcut(Input::KeyCode key, bool ctrl,
                                      bool shift, bool alt) noexcept;

    // ── Wiring helper ───────────────────────────────────────────────────

    void WireCallbacks() noexcept;

    // ── Members: services ───────────────────────────────────────────────

    WindowManager::DragonWindow* m_window{};
    Services m_services{};

    DragonUI::FocusManager m_focusManager;
    DragonUI::DialogManager m_dialogManager;
    DragonUI::AccessibilityManager m_accessibility;

    // ── Members: layout tree ────────────────────────────────────────────

    DragonUI::UIDockPanel m_root;
    DragonUI::UIStackPanel* m_topStack{};      ///< Tab strip + ribbon.
    DragonUI::UIDockPanel* m_centerRow{};      ///< Nav | center | preview.
    DragonUI::UIStackPanel* m_centerColumn{};  ///< Breadcrumb + list.

    std::unique_ptr<TabStrip> m_tabStrip;
    std::unique_ptr<RibbonBar> m_ribbon;
    std::unique_ptr<NavigationPane> m_navPane;     ///< Built in SetServices.
    std::unique_ptr<BreadcrumbBar> m_breadcrumb;
    std::unique_ptr<FileListPane> m_fileList;
    std::unique_ptr<PreviewPane> m_preview;
    std::unique_ptr<StatusBarPane> m_status;

    // ── Members: per-window state ───────────────────────────────────────

    BookmarkStore m_bookmarks;
    std::vector<Tab> m_tabs;
    size_t m_activeIndex{ 0 };
    uint64_t m_nextTabId{ 1 };
    std::vector<ClosedTab> m_closedTabs;

    std::unique_ptr<LoadPipeline> m_loader;        ///< Built in SetServices.
    std::unique_ptr<SearchController> m_search;    ///< Built in SetServices.
    ContextMenuService m_contextMenus;
    std::vector<std::unique_ptr<DragonUI::UIContextMenu>> m_openMenus;

    struct ClipboardEntry final {
        std::wstring sourcePath;
        bool isDirectory{};
    };
    std::vector<ClipboardEntry> m_clipboard;
    bool m_clipboardIsCut{ false };

    struct OpenWithEntry final {
        std::wstring extension;     ///< Lower-case with dot, e.g. L".txt".
        std::wstring label;
        std::function<void(const FileSystem::FileEntry&)> action;
    };
    std::vector<OpenWithEntry> m_openWithHandlers;

    DragonUI::Control* m_pressedControl{};              ///< Pump press state.
    std::vector<FileSystem::FileEntry> m_deleteTargets; ///< Pending delete.

    float m_clientX{}, m_clientY{}, m_clientW{}, m_clientH{};
    float m_viewportW{ 1280.0f }, m_viewportH{ 720.0f };
};

} // namespace Explorer2
} // namespace DragonOS
