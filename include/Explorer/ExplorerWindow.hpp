#pragma once

#include <Explorer/ExplorerTypes.hpp>

#include <FileSystem/FileEntry.hpp>
#include <Input/HitTest.hpp>
#include <UI/UI.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Graphics      { class Renderer; }
namespace DragonOS::Theme         { class ThemeManager; }
namespace DragonOS::Input         { class MouseManager; }
namespace DragonOS::WindowManager { class DragonWindow; }
namespace DragonOS::FileSystem    { class FileSystemService; }

namespace DragonOS::Explorer {

class ExplorerWindow final {
public:
    ExplorerWindow() noexcept;
    ~ExplorerWindow() noexcept;

    ExplorerWindow(const ExplorerWindow&)            = delete;
    ExplorerWindow& operator=(const ExplorerWindow&) = delete;
    ExplorerWindow(ExplorerWindow&&)                 = delete;
    ExplorerWindow& operator=(ExplorerWindow&&)      = delete;

    // ── Initialization ─────────────────────────────────────────────────

    void SetDependencies(
        WindowManager::DragonWindow&  window,
        FileSystem::FileSystemService& fsService,
        Theme::ThemeManager&          themeManager,
        Input::MouseManager&          mouseManager) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept
    {
        m_pWindow = &window;
    }

    // ── Navigation ─────────────────────────────────────────────────────

    void NavigateTo(const std::wstring& path) noexcept;
    void NavigateBack() noexcept;
    void NavigateForward() noexcept;
    void NavigateUp() noexcept;
    void Refresh() noexcept;
    void SetViewMode(ViewMode mode) noexcept;

    // ── Per-frame ──────────────────────────────────────────────────────

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;
    void ProcessInput() noexcept;

    // ── Accessors ──────────────────────────────────────────────────────

    [[nodiscard]] uint64_t           GetWindowId() const noexcept;
    [[nodiscard]] const std::wstring& GetCurrentPath() const noexcept { return m_currentPath; }
    [[nodiscard]] bool                CanGoBack() const noexcept { return m_historyIndex > 0; }
    [[nodiscard]] bool                CanGoForward() const noexcept { return m_historyIndex + 1 < m_history.size(); }

    // ── App info for StartMenu / Taskbar ───────────────────────────────

    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"Explorer";
        return name;
    }

private:
    // ── Layout ─────────────────────────────────────────────────────────

    void RecalculateLayout() noexcept;

    // ── Rendering ──────────────────────────────────────────────────────

    void RenderToolbar(Graphics::Renderer& renderer) noexcept;
    void RenderAddressBar(Graphics::Renderer& renderer) noexcept;
    void RenderNavigationPane(Graphics::Renderer& renderer) noexcept;
    void RenderFileView(Graphics::Renderer& renderer) noexcept;
    void RenderStatusBar(Graphics::Renderer& renderer) noexcept;
    void RenderContextMenu(Graphics::Renderer& renderer) noexcept;

    void RenderFileEntryGrid(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept;
    void RenderFileEntryList(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept;
    void RenderFileEntryDetails(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept;
    void RenderToolbarButton(Graphics::Renderer& renderer, const Input::Bounds& bounds, const std::wstring& label, bool hovered, bool pressed) noexcept;
    void RenderNavPaneIcon(Graphics::Renderer& renderer, const Input::Bounds& bounds, const std::wstring& label, bool hovered, uint32_t iconCode) noexcept;

    // ── Input ──────────────────────────────────────────────────────────

    [[nodiscard]] ExplorerHitRegion HitTest(float px, float py) const noexcept;
    [[nodiscard]] int               HitTestNavPane(float px, float py) const noexcept;
    [[nodiscard]] int               HitTestFileView(float px, float py) const noexcept;
    [[nodiscard]] int               HitTestContextMenu(float px, float py) const noexcept;
    [[nodiscard]] int               HitTestAddressBarSegment(float px, float py) const noexcept;

    void HandleToolbarClick(ExplorerHitRegion region) noexcept;
    void HandleNavPaneClick(int index) noexcept;
    void HandleFileViewClick(int index) noexcept;
    void HandleFileViewDoubleClick(int index) noexcept;
    void HandleAddressBarClick(int index) noexcept;
    void ShowContextMenu(float px, float py, size_t entryIndex) noexcept;
    void CloseContextMenu() noexcept;

    // ── Selection ──────────────────────────────────────────────────────

    void ClearSelection() noexcept;
    void SelectItem(size_t index, bool addToSelection = false) noexcept;
    void SelectRange(size_t from, size_t to) noexcept;

    // ── Icon cache ─────────────────────────────────────────────────────

    struct CachedIcon final {
        std::wstring extension;
        uint32_t     glyph{ 0 };
    };
    [[nodiscard]] uint32_t GetIconGlyph(const std::wstring& extension, bool isDirectory) noexcept;

    // ── Entry management ───────────────────────────────────────────────

    void LoadDirectory(const std::wstring& path) noexcept;
    void UpdateFileViewItems() noexcept;

    // ── UI Framework integration ──────────────────────────────────────

    void InitUIElements() noexcept;
    UI::UIRenderer MakeUIRenderer(Graphics::Renderer& renderer) const noexcept;

    std::unique_ptr<UI::Toolbar> m_uiToolbar;
    std::vector<UI::Button*>     m_uiToolbarButtons;
    std::unique_ptr<UI::StatusBar> m_uiStatusBar;
    std::unique_ptr<UI::ContextMenu> m_uiContextMenu;

    // ── Sub-components state ───────────────────────────────────────────

    ExplorerLayout  m_layout{};
    ViewMode        m_viewMode{ ViewMode::Details };

    // ── Navigation history ─────────────────────────────────────────────

    std::vector<std::wstring> m_history;
    size_t                    m_historyIndex{ 0 };

    // ── Current directory state ────────────────────────────────────────

    std::wstring               m_currentPath;
    std::vector<FileSystem::FileEntry> m_entries;
    bool                       m_entriesLoaded{ false };

    // ── File view items (cached bounds for hit testing) ────────────────

    std::vector<FileViewItem> m_fileViewItems;

    // ── Navigation pane ────────────────────────────────────────────────

    std::vector<NavPaneEntry> m_navPaneEntries;
    int                       m_hoveredNavIdx{ -1 };
    int                       m_pressedNavIdx{ -1 };

    // ── Toolbar state ──────────────────────────────────────────────────

    ExplorerHitRegion m_toolbarHover{ ExplorerHitRegion::None };
    ExplorerHitRegion m_toolbarPressed{ ExplorerHitRegion::None };

    // ── Address bar state ──────────────────────────────────────────────

    int m_hoveredAddressIdx{ -1 };

    // ── File view state ────────────────────────────────────────────────

    int  m_hoveredFileIdx{ -1 };
    std::vector<size_t> m_selectedIndices;
    bool m_isSelecting{ false };
    int  m_selectAnchor{ -1 };

    // ── Context menu ───────────────────────────────────────────────────

    bool                 m_contextMenuOpen{ false };
    size_t               m_contextMenuTargetEntry{ static_cast<size_t>(-1) };

    // ── Scroll ─────────────────────────────────────────────────────────

    float m_scrollOffset{ 0.0f };
    float m_scrollTarget{ 0.0f };
    float m_scrollVelocity{ 0.0f };

    // ── Icon cache (extension → glyph) ─────────────────────────────────

    std::vector<CachedIcon> m_iconCache;

    // ── Non-owning references ──────────────────────────────────────────

    WindowManager::DragonWindow*   m_pWindow{ nullptr };
    FileSystem::FileSystemService* m_pFS{ nullptr };
    Theme::ThemeManager*           m_pTheme{ nullptr };
    Input::MouseManager*           m_pMouse{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Explorer
