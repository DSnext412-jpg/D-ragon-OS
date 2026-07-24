#include <Explorer/ExplorerWindow.hpp>

#include <FileSystem/FileSystemService.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Theme/ThemePalette.hpp>
#include <WindowManager/DragonWindow.hpp>

#include <algorithm>
#include <cwchar>
#include <d2d1.h>

namespace DragonOS::Explorer {

// ============================================================================
//  Construction / Destruction
// ============================================================================

ExplorerWindow::ExplorerWindow() noexcept = default;

ExplorerWindow::~ExplorerWindow() noexcept
{
    m_pWindow = nullptr;
    m_pFS     = nullptr;
    m_pTheme  = nullptr;
    m_pMouse  = nullptr;
}

// ============================================================================
//  Initialization
// ============================================================================

void ExplorerWindow::SetDependencies(
    WindowManager::DragonWindow&  window,
    FileSystem::FileSystemService& fsService,
    Theme::ThemeManager&          themeManager,
    Input::MouseManager&          mouseManager) noexcept
{
    m_pWindow = &window;
    m_pFS     = &fsService;
    m_pTheme  = &themeManager;
    m_pMouse  = &mouseManager;

    // Build navigation pane entries
    m_navPaneEntries.clear();
    const auto addNavEntry = [this](const std::wstring& name, FileSystem::KnownFolder folder, uint32_t glyph) {
        NavPaneEntry entry;
        entry.displayName = name;
        entry.path = m_pFS->GetKnownFolderPath(folder);
        entry.iconGlyph = glyph;
        m_navPaneEntries.push_back(std::move(entry));
    };

    addNavEntry(L"Home",       FileSystem::KnownFolder::Home,     0x1F3E0);
    addNavEntry(L"Desktop",    FileSystem::KnownFolder::Desktop,   0x1F4BB);
    addNavEntry(L"Documents",  FileSystem::KnownFolder::Documents, 0x1F4C4);
    addNavEntry(L"Downloads",  FileSystem::KnownFolder::Downloads, 0x1F4E5);
    addNavEntry(L"Pictures",   FileSystem::KnownFolder::Pictures,  0x1F5BC);
    addNavEntry(L"Music",      FileSystem::KnownFolder::Music,     0x1F3B5);
    addNavEntry(L"Videos",     FileSystem::KnownFolder::Videos,    0x1F3AC);

    // This PC entry (shows drives)
    {
        NavPaneEntry entry;
        entry.displayName = L"This PC";
        entry.path = L"";
        entry.iconGlyph = 0x1F4BE;
        m_navPaneEntries.push_back(std::move(entry));
    }

    // Recycle Bin (placeholder)
    {
        NavPaneEntry entry;
        entry.displayName = L"Recycle Bin";
        entry.path = L"";
        entry.iconGlyph = 0x1F5D1;
        m_navPaneEntries.push_back(std::move(entry));
    }

    // ── Initialize UI Framework elements ──────────────────────────
    InitUIElements();

    // Start at Desktop
    const auto desktopPath = m_pFS->GetKnownFolderPath(FileSystem::KnownFolder::Desktop);
    NavigateTo(desktopPath);

    m_initialized = true;
}

// ============================================================================
//  UI Framework Element Initialization
// ============================================================================

void ExplorerWindow::InitUIElements() noexcept
{
    // ── Toolbar with buttons ──────────────────────────────────────────
    m_uiToolbar = std::make_unique<UI::Toolbar>();
    m_uiToolbarButtons.clear();

    struct ToolbarButtonDef {
        std::wstring text;
        wchar_t icon;
        ExplorerHitRegion region;
    };

    const ToolbarButtonDef defs[] = {
        {L"", 0x25C0, ExplorerHitRegion::ToolbarBack},
        {L"", 0x25B6, ExplorerHitRegion::ToolbarForward},
        {L"", 0x2191, ExplorerHitRegion::ToolbarUp},
        {L"", 0x21BB, ExplorerHitRegion::ToolbarRefresh},
        {L"", 0x2795, ExplorerHitRegion::ToolbarNewFolder},
        {L"", 0x2702, ExplorerHitRegion::ToolbarCopy},
        {L"", 0x2702, ExplorerHitRegion::ToolbarPaste},
        {L"", 0x2716, ExplorerHitRegion::ToolbarDelete},
        {L"", 0x270E, ExplorerHitRegion::ToolbarRename},
        {L"", 0x2699, ExplorerHitRegion::ToolbarProperties},
    };

    for (const auto& def : defs)
    {
        auto* btn = m_uiToolbar->AddButton(def.text, def.icon,
            [this, region = def.region]() {
                HandleToolbarClick(region);
            });
        m_uiToolbarButtons.push_back(btn);
    }

    // ── Status bar ────────────────────────────────────────────────────
    m_uiStatusBar = std::make_unique<UI::StatusBar>();

    // ── Context menu ──────────────────────────────────────────────────
    m_uiContextMenu = std::make_unique<UI::ContextMenu>();
}

UI::UIRenderer ExplorerWindow::MakeUIRenderer(Graphics::Renderer& renderer) const noexcept
{
    return UI::UIRenderer(renderer, *m_pTheme);
}

// ============================================================================
//  Navigation
// ============================================================================

void ExplorerWindow::NavigateTo(const std::wstring& path) noexcept
{
    if (!m_pFS) { return; }

    // Truncate future history when navigating to a new location
    if (m_historyIndex + 1 < m_history.size())
    {
        m_history.resize(m_historyIndex + 1);
    }

    m_history.push_back(path);
    m_historyIndex = m_history.size() - 1;

    m_selectedIndices.clear();
    m_scrollOffset = 0.0f;
    m_contextMenuOpen = false;

    LoadDirectory(path);
}

void ExplorerWindow::NavigateBack() noexcept
{
    if (!CanGoBack()) { return; }
    --m_historyIndex;
    m_selectedIndices.clear();
    m_scrollOffset = 0.0f;
    m_contextMenuOpen = false;
    LoadDirectory(m_history[m_historyIndex]);
}

void ExplorerWindow::NavigateForward() noexcept
{
    if (!CanGoForward()) { return; }
    ++m_historyIndex;
    m_selectedIndices.clear();
    m_scrollOffset = 0.0f;
    m_contextMenuOpen = false;
    LoadDirectory(m_history[m_historyIndex]);
}

void ExplorerWindow::NavigateUp() noexcept
{
    if (!m_pFS) { return; }
    const std::wstring parent = m_pFS->GetParentPath(m_currentPath);
    if (!parent.empty() && parent != m_currentPath)
    {
        NavigateTo(parent);
    }
}

void ExplorerWindow::Refresh() noexcept
{
    if (!m_currentPath.empty())
    {
        LoadDirectory(m_currentPath);
    }
}

void ExplorerWindow::SetViewMode(ViewMode mode) noexcept
{
    m_viewMode = mode;
    UpdateFileViewItems();
}

// ============================================================================
//  Directory loading
// ============================================================================

void ExplorerWindow::LoadDirectory(const std::wstring& path) noexcept
{
    if (!m_pFS) { return; }

    m_currentPath = path;
    m_entries.clear();
    m_fileViewItems.clear();
    m_entriesLoaded = false;

    auto result = m_pFS->ListDirectory(path);
    if (result.success)
    {
        m_entries = std::move(result.entries);
    }

    m_entriesLoaded = true;
    UpdateFileViewItems();
}

void ExplorerWindow::UpdateFileViewItems() noexcept
{
    m_fileViewItems.clear();
    RecalculateLayout();

    const ExplorerLayout& lay = m_layout;
    if (m_entries.empty()) { return; }

    if (m_viewMode == ViewMode::Grid)
    {
        const float itemSize = ExplorerLayout::GridItemSize;
        const float gap = ExplorerLayout::GridGap;
        const int cols = (std::max)(1, static_cast<int>((lay.fileViewArea.width - gap) / (itemSize + gap)));

        float x = lay.fileViewArea.x + gap;
        float y = lay.fileViewArea.y + gap - m_scrollOffset;

        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            const int row = static_cast<int>(i) / cols;
            const int col = static_cast<int>(i) % cols;

            FileViewItem item;
            item.entryIndex = i;
            item.bounds = {
                x + col * (itemSize + gap),
                y + row * (itemSize + gap + 20.0f),
                itemSize,
                itemSize + 20.0f
            };
            m_fileViewItems.push_back(std::move(item));
        }

        if (!m_fileViewItems.empty())
        {
            const auto& last = m_fileViewItems.back();
            const float totalH = (last.bounds.y + last.bounds.height) - lay.fileViewArea.y + gap;
            m_layout.scrollContentHeight = totalH;
        }
    }
    else if (m_viewMode == ViewMode::List)
    {
        const float itemH = ExplorerLayout::ListItemHeight;
        float y = lay.fileViewArea.y + ExplorerLayout::SectionPadding - m_scrollOffset;

        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            FileViewItem item;
            item.entryIndex = i;
            item.bounds = {
                lay.fileViewArea.x + 4.0f,
                y,
                lay.fileViewArea.width - 8.0f,
                itemH
            };
            m_fileViewItems.push_back(std::move(item));
            y += itemH;
        }

        if (!m_fileViewItems.empty())
        {
            const auto& last = m_fileViewItems.back();
            m_layout.scrollContentHeight = (last.bounds.y + last.bounds.height) - lay.fileViewArea.y + ExplorerLayout::SectionPadding;
        }
    }
    else // Details
    {
        const float itemH = ExplorerLayout::DetailsItemHeight;
        float y = lay.fileViewArea.y + ExplorerLayout::SectionPadding + itemH - m_scrollOffset;

        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            FileViewItem item;
            item.entryIndex = i;
            item.bounds = {
                lay.fileViewArea.x + 4.0f,
                y,
                lay.fileViewArea.width - 8.0f,
                itemH
            };
            m_fileViewItems.push_back(std::move(item));
            y += itemH;
        }

        if (!m_fileViewItems.empty())
        {
            const auto& last = m_fileViewItems.back();
            m_layout.scrollContentHeight = (last.bounds.y + last.bounds.height) - lay.fileViewArea.y + ExplorerLayout::SectionPadding + itemH;
        }
    }

    m_layout.scrollMaxOffset = (std::max)(0.0f, m_layout.scrollContentHeight - (lay.fileViewArea.height - ExplorerLayout::SectionPadding));
    if (m_scrollOffset > m_layout.scrollMaxOffset)
    {
        m_scrollOffset = m_layout.scrollMaxOffset;
    }
}

// ============================================================================
//  Layout
// ============================================================================

void ExplorerWindow::RecalculateLayout() noexcept
{
    if (!m_pWindow) { return; }

    const float border = Theme::ThemeMetrics::WindowBorderThickness;
    const float titleH = Theme::ThemeMetrics::TitleBarHeight;

    // Client area (relative to window origin)
    m_layout.clientArea = {
        m_pWindow->GetX() + border,
        m_pWindow->GetY() + titleH,
        m_pWindow->GetWidth() - 2.0f * border,
        m_pWindow->GetHeight() - titleH - border
    };

    const float cx = m_layout.clientArea.x;
    const float cy = m_layout.clientArea.y;
    const float cw = m_layout.clientArea.width;
    const float ch = m_layout.clientArea.height;

    const float toolbarH = 36.0f;
    const float addrBarH = 32.0f;
    const float statusH  = 24.0f;
    const float btnSize  = 26.0f;
    const float btnGap   = 2.0f;

    // Toolbar
    m_layout.toolbarArea = { cx + 4.0f, cy + 4.0f, cw - 8.0f, toolbarH };

    float tx = cx + 4.0f;
    const float ty = cy + 6.0f;
    m_layout.btnBack       = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnForward    = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnUp         = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    tx += 4.0f;
    m_layout.btnRefresh    = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnNewFolder  = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    tx += 4.0f;
    m_layout.btnCopy       = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnPaste      = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnDelete     = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnRename     = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;
    m_layout.btnProperties = { tx, ty, btnSize, btnSize }; tx += btnSize + btnGap;

    // Address bar
    m_layout.addressBarArea = { cx + 4.0f, cy + toolbarH + 8.0f, cw - 8.0f, addrBarH };

    // Navigation pane
    m_layout.navPaneWidth = 180.0f;
    m_layout.navPaneArea = {
        cx + 4.0f,
        m_layout.addressBarArea.y + m_layout.addressBarArea.height + 4.0f,
        m_layout.navPaneWidth,
        ch - toolbarH - addrBarH - statusH - 16.0f
    };

    // File view
    m_layout.fileViewArea = {
        m_layout.navPaneArea.Right() + 4.0f,
        m_layout.navPaneArea.y,
        cw - m_layout.navPaneWidth - 12.0f,
        m_layout.navPaneArea.height
    };

    // Status bar
    m_layout.statusBarArea = {
        cx + 4.0f,
        cy + ch - statusH - 4.0f,
        cw - 8.0f,
        statusH
    };

    // ── UI Framework element layout ───────────────────────────────────
    if (m_uiToolbar)
    {
        D2D1_RECT_F tbBounds = {
            cx + 4.0f, cy + 4.0f,
            cx + cw - 8.0f, cy + 4.0f + toolbarH
        };
        m_uiToolbar->Measure(D2D1::RectF(0, 0, cw - 8.0f, toolbarH));
        m_uiToolbar->Arrange(tbBounds);
    }

    if (m_uiStatusBar)
    {
        D2D1_RECT_F sbBounds = {
            m_layout.statusBarArea.x, m_layout.statusBarArea.y,
            m_layout.statusBarArea.Right(), m_layout.statusBarArea.Bottom()
        };
        m_uiStatusBar->Measure(D2D1::RectF(0, 0, cw - 8.0f, statusH));
        m_uiStatusBar->Arrange(sbBounds);
    }

    m_layout.scrollOffset = m_scrollOffset;
}

// ============================================================================
//  Per-frame
// ============================================================================

void ExplorerWindow::Update() noexcept
{
    if (!m_initialized || !m_pWindow) { return; }

    // Only process if window is visible
    if (!m_pWindow->IsVisible()) { return; }

    // Smooth scroll
    if (std::abs(m_scrollVelocity) > 0.5f)
    {
        m_scrollOffset += m_scrollVelocity * 0.016f;
        m_scrollVelocity *= 0.85f;
        if (std::abs(m_scrollVelocity) < 0.5f) { m_scrollVelocity = 0.0f; }

        m_scrollOffset = (std::max)(0.0f, (std::min)(m_scrollOffset, m_layout.scrollMaxOffset));
        UpdateFileViewItems();
    }

    ProcessInput();
}

void ExplorerWindow::ProcessInput() noexcept
{
    if (!m_initialized || !m_pMouse || !m_pWindow) { return; }
    if (!m_pWindow->IsVisible()) { return; }

    const auto pos = m_pMouse->GetPosition();
    const auto& client = m_layout.clientArea;

    // Handle context menu via UI framework
    if (m_contextMenuOpen && m_uiContextMenu)
    {
        UI::UIEvent cmEvent;
        cmEvent.x = pos.x;
        cmEvent.y = pos.y;
        cmEvent.button = Input::MouseButton::Left;

        // Check if mouse is over the context menu
        UI::UIElement* hit = m_uiContextMenu->HitTest(pos.x, pos.y);
        if (hit)
        {
            // Mouse move over context menu
            cmEvent.type = UI::UIEventType::MouseMove;
            m_uiContextMenu->OnEvent(cmEvent);

            if (m_pMouse->WasLeftClicked())
            {
                cmEvent.type = UI::UIEventType::Click;
                m_uiContextMenu->OnEvent(cmEvent);
                CloseContextMenu();
                return; // Click handled
            }
        }
        else
        {
            if (m_pMouse->WasLeftClicked())
            {
                CloseContextMenu();
            }
        }
    }

    if (!client.Contains(pos.x, pos.y))
    {
        m_toolbarHover = ExplorerHitRegion::None;
        m_hoveredNavIdx = -1;
        m_hoveredFileIdx = -1;
        return;
    }

    // Hit-test sub-components
    const auto hitRegion = HitTest(pos.x, pos.y);

    // Toolbar hover
    m_toolbarHover = hitRegion;

    // Navigation pane hover
    if (hitRegion == ExplorerHitRegion::NavPaneItem)
    {
        m_hoveredNavIdx = HitTestNavPane(pos.x, pos.y);
    }
    else
    {
        m_hoveredNavIdx = -1;
    }

    // File view hover
    if (hitRegion == ExplorerHitRegion::FileViewItem)
    {
        m_hoveredFileIdx = HitTestFileView(pos.x, pos.y);
    }
    else
    {
        m_hoveredFileIdx = -1;
    }

    // ── Click handling ────────────────────────────────────────────────
    if (m_pMouse->WasLeftClicked())
    {
        m_toolbarPressed = hitRegion;

        switch (hitRegion)
        {
        case ExplorerHitRegion::ToolbarBack:
        case ExplorerHitRegion::ToolbarForward:
        case ExplorerHitRegion::ToolbarUp:
        case ExplorerHitRegion::ToolbarRefresh:
        case ExplorerHitRegion::ToolbarNewFolder:
        case ExplorerHitRegion::ToolbarCopy:
        case ExplorerHitRegion::ToolbarPaste:
        case ExplorerHitRegion::ToolbarDelete:
        case ExplorerHitRegion::ToolbarRename:
        case ExplorerHitRegion::ToolbarProperties:
            HandleToolbarClick(hitRegion);
            break;

        case ExplorerHitRegion::NavPaneItem:
            HandleNavPaneClick(m_hoveredNavIdx);
            break;

        case ExplorerHitRegion::FileViewItem:
            HandleFileViewClick(m_hoveredFileIdx);
            break;

        case ExplorerHitRegion::FileViewBackground:
            ClearSelection();
            CloseContextMenu();
            break;

        default:
            break;
        }
    }

    // ── Double-click ──────────────────────────────────────────────────
    if (m_pMouse->WasDoubleClicked(Input::MouseButton::Left))
    {
        if (hitRegion == ExplorerHitRegion::FileViewItem && m_hoveredFileIdx >= 0)
        {
            HandleFileViewDoubleClick(m_hoveredFileIdx);
        }
    }

    // ── Right-click ───────────────────────────────────────────────────
    if (m_pMouse->WasRightClicked())
    {
        if (hitRegion == ExplorerHitRegion::FileViewItem && m_hoveredFileIdx >= 0)
        {
            ShowContextMenu(pos.x, pos.y, m_fileViewItems[m_hoveredFileIdx].entryIndex);
        }
        else if (hitRegion == ExplorerHitRegion::FileViewBackground)
        {
            ShowContextMenu(pos.x, pos.y, static_cast<size_t>(-1));
        }
    }

    // ── Release ───────────────────────────────────────────────────────
    if (m_pMouse->WasReleased(Input::MouseButton::Left))
    {
        m_toolbarPressed = ExplorerHitRegion::None;
    }

    // ── Scroll wheel ──────────────────────────────────────────────────
    const float wheelDelta = m_pMouse->GetWheelDelta();
    if (std::abs(wheelDelta) > 0.5f)
    {
        m_scrollOffset -= wheelDelta * 20.0f;
        m_scrollOffset = (std::max)(0.0f, (std::min)(m_scrollOffset, m_layout.scrollMaxOffset));
        UpdateFileViewItems();
    }
}

// ============================================================================
//  Rendering
// ============================================================================

void ExplorerWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized || !m_pWindow || !m_pTheme) { return; }
    if (!m_pWindow->IsVisible()) { return; }

    RecalculateLayout();

    const auto& client = m_layout.clientArea;

    // Client area background
    const auto& bgCol = m_pTheme->GetColor(Theme::SemanticColor::WindowBackground);
    const D2D1_RECT_F clientRect = D2D1::RectF(client.x, client.y, client.Right(), client.Bottom());
    renderer.FillRectangle(clientRect, Graphics::Color{ bgCol.r, bgCol.g, bgCol.b, bgCol.a });

    // Render sub-components
    RenderToolbar(renderer);
    RenderAddressBar(renderer);
    RenderNavigationPane(renderer);
    RenderFileView(renderer);
    RenderStatusBar(renderer);

    // Context menu (on top)
    if (m_contextMenuOpen)
    {
        RenderContextMenu(renderer);
    }
}

// ============================================================================
//  Toolbar (UI Framework)
// ============================================================================

void ExplorerWindow::RenderToolbar(Graphics::Renderer& renderer) noexcept
{
    if (!m_uiToolbar || !m_pTheme) return;

    // Sync button states from toolbar hit region tracking
    const ExplorerHitRegion regions[] = {
        ExplorerHitRegion::ToolbarBack, ExplorerHitRegion::ToolbarForward,
        ExplorerHitRegion::ToolbarUp, ExplorerHitRegion::ToolbarRefresh,
        ExplorerHitRegion::ToolbarNewFolder, ExplorerHitRegion::ToolbarCopy,
        ExplorerHitRegion::ToolbarPaste, ExplorerHitRegion::ToolbarDelete,
        ExplorerHitRegion::ToolbarRename, ExplorerHitRegion::ToolbarProperties,
    };

    for (size_t i = 0; i < m_uiToolbarButtons.size() && i < 10; ++i) // 10 toolbar buttons
    {
        auto* btn = m_uiToolbarButtons[i];
        if (!btn) continue;

        bool isHover = (m_toolbarHover == regions[i]);
        bool isPressed = (m_toolbarPressed == regions[i]);

        if (isPressed)
            btn->SetState(UI::ElementState::Pressed);
        else if (isHover)
            btn->SetState(UI::ElementState::Hover);
        else
            btn->SetState(UI::ElementState::Normal);
    }

    auto uiRenderer = MakeUIRenderer(renderer);
    m_uiToolbar->Render(uiRenderer);
}

// ============================================================================
//  Address Bar
// ============================================================================

void ExplorerWindow::RenderAddressBar(Graphics::Renderer& renderer) noexcept
{
    const auto& bgCol = m_pTheme->GetColor(Theme::SemanticColor::WindowBackground);

    // Address bar background
    const auto& addr = m_layout.addressBarArea;
    const D2D1_RECT_F addrRect = D2D1::RectF(addr.x, addr.y, addr.Right(), addr.Bottom());

    // Slightly darker background for the address bar
    const Graphics::Color addrBg{ bgCol.r * 0.85f, bgCol.g * 0.85f, bgCol.b * 0.85f, bgCol.a };
    renderer.FillRectangle(addrRect, addrBg);

    // Draw breadcrumb text
    const auto& textCol = m_pTheme->GetColor(Theme::SemanticColor::TextPrimary);
    const D2D1_RECT_F textRect = D2D1::RectF(addr.x + 8.0f, addr.y + 4.0f, addr.Right() - 4.0f, addr.Bottom() - 4.0f);
    renderer.DrawText(m_currentPath, textRect, Graphics::Color{ textCol.r, textCol.g, textCol.b, textCol.a });
}

// ============================================================================
//  Navigation Pane
// ============================================================================

void ExplorerWindow::RenderNavigationPane(Graphics::Renderer& renderer) noexcept
{
    const auto& navArea = m_layout.navPaneArea;
    const auto& navBgCol = m_pTheme->GetColor(Theme::SemanticColor::WindowBackground);

    // Navigation pane background (slightly different shade)
    const D2D1_RECT_F navRect = D2D1::RectF(navArea.x, navArea.y, navArea.Right(), navArea.Bottom());
    const Graphics::Color navBg{ navBgCol.r * 0.92f, navBgCol.g * 0.92f, navBgCol.b * 0.92f, navBgCol.a };
    renderer.FillRectangle(navRect, navBg);

    // Draw entries
    const auto& textCol = m_pTheme->GetColor(Theme::SemanticColor::TextPrimary);
    const auto& hoverCol = m_pTheme->GetColor(Theme::SemanticColor::Hover);
    const auto& selectedCol = m_pTheme->GetColor(Theme::SemanticColor::Selection);

    float ey = navArea.y + 4.0f;
    const float itemH = 32.0f;
    const float iconSize = 18.0f;

    for (int i = 0; i < static_cast<int>(m_navPaneEntries.size()); ++i)
    {
        const Input::Bounds entryBounds{ navArea.x + 2.0f, ey, navArea.width - 4.0f, itemH };
        const D2D1_RECT_F entryRect = D2D1::RectF(entryBounds.x, entryBounds.y, entryBounds.Right(), entryBounds.Bottom());

        const bool selected = m_navPaneEntries[i].path == m_currentPath;

        if (selected)
        {
            renderer.FillRectangle(entryRect, Graphics::Color{ selectedCol.r, selectedCol.g, selectedCol.b, selectedCol.a });
        }
        else if (i == m_hoveredNavIdx)
        {
            renderer.FillRectangle(entryRect, Graphics::Color{ hoverCol.r, hoverCol.g, hoverCol.b, hoverCol.a });
        }

        // Icon
        const D2D1_RECT_F iconRect = D2D1::RectF(entryBounds.x + 4.0f, entryBounds.y + (itemH - iconSize) / 2.0f,
                                                   entryBounds.x + 4.0f + iconSize, entryBounds.y + (itemH - iconSize) / 2.0f + iconSize);
        // Draw a placeholder box as icon
        const auto& accentCol = m_pTheme->GetColor(Theme::SemanticColor::Accent);
        renderer.FillRectangle(iconRect, Graphics::Color{ accentCol.r, accentCol.g, accentCol.b, 0.3f });

        // Label
        const D2D1_RECT_F labelRect = D2D1::RectF(entryBounds.x + 26.0f, entryBounds.y + 2.0f,
                                                    entryBounds.Right() - 4.0f, entryBounds.Bottom() - 2.0f);
        renderer.DrawText(m_navPaneEntries[i].displayName, labelRect, Graphics::Color{ textCol.r, textCol.g, textCol.b, textCol.a });

        ey += itemH;
    }
}

// ============================================================================
//  File View
// ============================================================================

void ExplorerWindow::RenderFileView(Graphics::Renderer& renderer) noexcept
{
    const auto& fv = m_layout.fileViewArea;

    const D2D1_RECT_F fvRect = D2D1::RectF(fv.x, fv.y, fv.Right(), fv.Bottom());

    if (m_viewMode == ViewMode::Details)
    {
        // Column headers
        const auto& headerCol = m_pTheme->GetColor(Theme::SemanticColor::WindowTitleBar);
        const auto& headerTextCol = m_pTheme->GetColor(Theme::SemanticColor::TextSecondary);

        const float colName = fv.x + 4.0f;
        const float colSize = fv.x + fv.width * 0.35f;
        const float colType = fv.x + fv.width * 0.60f;

        const D2D1_RECT_F headerRect = D2D1::RectF(fv.x, fv.y, fv.Right(), fv.y + ExplorerLayout::DetailsItemHeight);
        renderer.FillRectangle(headerRect, Graphics::Color{ headerCol.r, headerCol.g, headerCol.b, headerCol.a });

        renderer.DrawText(L"Name", D2D1::RectF(colName, fv.y + 2.0f, colSize, fv.y + ExplorerLayout::DetailsItemHeight), Graphics::Color{ headerTextCol.r, headerTextCol.g, headerTextCol.b, headerTextCol.a });
        renderer.DrawText(L"Size", D2D1::RectF(colSize + 4.0f, fv.y + 2.0f, colType, fv.y + ExplorerLayout::DetailsItemHeight), Graphics::Color{ headerTextCol.r, headerTextCol.g, headerTextCol.b, headerTextCol.a });
        renderer.DrawText(L"Date Modified", D2D1::RectF(colType + 4.0f, fv.y + 2.0f, fv.Right(), fv.y + ExplorerLayout::DetailsItemHeight), Graphics::Color{ headerTextCol.r, headerTextCol.g, headerTextCol.b, headerTextCol.a });
    }

    // Render each visible file entry
    const auto& hoverCol = m_pTheme->GetColor(Theme::SemanticColor::Hover);
    const auto& selBgCol = m_pTheme->GetColor(Theme::SemanticColor::Selection);

    for (size_t i = 0; i < m_fileViewItems.size(); ++i)
    {
        const auto& item = m_fileViewItems[i];
        const size_t entryIdx = item.entryIndex;

        if (entryIdx >= m_entries.size()) { continue; }

        const D2D1_RECT_F itemRect = D2D1::RectF(item.bounds.x, item.bounds.y, item.bounds.Right(), item.bounds.Bottom());

        // Check visibility
        if (item.bounds.y > m_layout.fileViewArea.Bottom() || item.bounds.Bottom() < m_layout.fileViewArea.y)
        {
            continue;
        }

        const bool selected = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), entryIdx) != m_selectedIndices.end();
        const bool hovered = (static_cast<int>(i) == m_hoveredFileIdx);
        const bool denied = m_entries[entryIdx].accessDenied;

        if (denied)
        {
            const auto& deniedBg = m_pTheme->GetColor(Theme::SemanticColor::Disabled);
            renderer.FillRectangle(itemRect, Graphics::Color{ deniedBg.r, deniedBg.g, deniedBg.b, 0.3f });
        }
        else if (selected)
        {
            renderer.FillRectangle(itemRect, Graphics::Color{ selBgCol.r, selBgCol.g, selBgCol.b, selBgCol.a });
        }
        else if (hovered)
        {
            renderer.FillRectangle(itemRect, Graphics::Color{ hoverCol.r, hoverCol.g, hoverCol.b, hoverCol.a });
        }

        switch (m_viewMode)
        {
        case ViewMode::Grid:
            RenderFileEntryGrid(renderer, entryIdx, item.bounds);
            break;
        case ViewMode::List:
            RenderFileEntryList(renderer, entryIdx, item.bounds);
            break;
        case ViewMode::Details:
            RenderFileEntryDetails(renderer, entryIdx, item.bounds);
            break;
        }
    }

    // Empty state
    if (m_entries.empty())
    {
        const auto& secCol = m_pTheme->GetColor(Theme::SemanticColor::TextSecondary);
        const D2D1_RECT_F emptyRect = D2D1::RectF(fv.x + 16.0f, fv.y + 40.0f, fv.Right() - 16.0f, fv.y + 80.0f);
        renderer.DrawText(L"This folder is empty", emptyRect, Graphics::Color{ secCol.r, secCol.g, secCol.b, secCol.a });
    }
}

void ExplorerWindow::RenderFileEntryGrid(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept
{
    if (index >= m_entries.size()) { return; }
    const auto& entry = m_entries[index];
    const auto& textCol = m_pTheme->GetColor(Theme::SemanticColor::TextPrimary);
    const auto& accentCol = m_pTheme->GetColor(Theme::SemanticColor::Accent);

    // Icon area (top portion)
    const float iconSize = 48.0f;
    const float iconX = bounds.x + (bounds.width - iconSize) / 2.0f;
    const float iconY = bounds.y + 4.0f;

    const D2D1_RECT_F iconRect = D2D1::RectF(iconX, iconY, iconX + iconSize, iconY + iconSize);
    const Graphics::Color iconBg{ accentCol.r, accentCol.g, accentCol.b, 0.2f };
    renderer.FillRectangle(iconRect, iconBg);

    // Label
    const float labelY = iconY + iconSize + 2.0f;
    const D2D1_RECT_F labelRect = D2D1::RectF(bounds.x + 2.0f, labelY, bounds.Right() - 2.0f, bounds.Bottom() - 2.0f);

    if (entry.accessDenied)
    {
        const auto& deniedCol = m_pTheme->GetColor(Theme::SemanticColor::Disabled);
        renderer.DrawText(entry.name, labelRect, Graphics::Color{ deniedCol.r, deniedCol.g, deniedCol.b, deniedCol.a });

        const std::wstring lockIcon = L"\U0001F512";
        const D2D1_RECT_F lockRect = D2D1::RectF(bounds.Right() - 20.0f, bounds.y + 2.0f, bounds.Right() - 2.0f, bounds.y + 20.0f);
        renderer.DrawText(lockIcon, lockRect, Graphics::Color{ 1, 0.3f, 0.3f, 1 });
    }
    else
    {
        renderer.DrawText(entry.name, labelRect, Graphics::Color{ textCol.r, textCol.g, textCol.b, textCol.a });
    }
}

void ExplorerWindow::RenderFileEntryList(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept
{
    if (index >= m_entries.size()) { return; }
    const auto& entry = m_entries[index];
    const auto& textCol = m_pTheme->GetColor(Theme::SemanticColor::TextPrimary);

    // Icon
    constexpr float iconSize = 16.0f;
    const D2D1_RECT_F iconRect = D2D1::RectF(bounds.x + 2.0f, bounds.y + (bounds.height - iconSize) / 2.0f,
                                              bounds.x + 2.0f + iconSize, bounds.y + (bounds.height - iconSize) / 2.0f + iconSize);
    const auto& accentCol = m_pTheme->GetColor(Theme::SemanticColor::Accent);
    renderer.FillRectangle(iconRect, Graphics::Color{ accentCol.r, accentCol.g, accentCol.b, 0.3f });

    // Name
    const D2D1_RECT_F nameRect = D2D1::RectF(bounds.x + 22.0f, bounds.y + 1.0f, bounds.Right() - 4.0f, bounds.Bottom() - 1.0f);

    if (entry.accessDenied)
    {
        const auto& deniedCol = m_pTheme->GetColor(Theme::SemanticColor::Disabled);
        renderer.DrawText(entry.name, nameRect, Graphics::Color{ deniedCol.r, deniedCol.g, deniedCol.b, deniedCol.a });
        renderer.DrawText(L"\U0001F512", D2D1::RectF(bounds.Right() - 20.0f, bounds.y + 1.0f, bounds.Right() - 2.0f, bounds.Bottom() - 1.0f), Graphics::Color{ 1, 0.3f, 0.3f, 1 });
    }
    else
    {
        renderer.DrawText(entry.name, nameRect, Graphics::Color{ textCol.r, textCol.g, textCol.b, textCol.a });
    }
}

void ExplorerWindow::RenderFileEntryDetails(Graphics::Renderer& renderer, size_t index, const Input::Bounds& bounds) noexcept
{
    if (index >= m_entries.size()) { return; }
    const auto& entry = m_entries[index];
    const auto& textCol = m_pTheme->GetColor(Theme::SemanticColor::TextPrimary);
    const auto& secCol = m_pTheme->GetColor(Theme::SemanticColor::TextSecondary);

    const auto& fv = m_layout.fileViewArea;
    const float colName  = fv.x + 4.0f;
    const float colSize  = fv.x + fv.width * 0.35f;
    const float colType  = fv.x + fv.width * 0.60f;

    // Name
    const D2D1_RECT_F nameRect = D2D1::RectF(colName, bounds.y + 1.0f, colSize, bounds.Bottom() - 1.0f);
    renderer.DrawText(entry.name, nameRect, Graphics::Color{ textCol.r, textCol.g, textCol.b, textCol.a });

    // Size
    if (!entry.IsDirectory())
    {
        const std::wstring sizeStr = FileSystem::FileSystemService::FormatFileSize(entry.size);
        const D2D1_RECT_F sizeRect = D2D1::RectF(colSize + 4.0f, bounds.y + 1.0f, colType, bounds.Bottom() - 1.0f);
        renderer.DrawText(sizeStr, sizeRect, Graphics::Color{ secCol.r, secCol.g, secCol.b, secCol.a });
    }

    // Date Modified
    const std::wstring dateStr = FileSystem::FileSystemService::FormatDateTime(entry.lastModified);
    const D2D1_RECT_F dateRect = D2D1::RectF(colType + 4.0f, bounds.y + 1.0f, fv.Right() - 4.0f, bounds.Bottom() - 1.0f);
    renderer.DrawText(dateStr, dateRect, Graphics::Color{ secCol.r, secCol.g, secCol.b, secCol.a });
}

// ============================================================================
//  Status Bar
// ============================================================================

void ExplorerWindow::RenderStatusBar(Graphics::Renderer& renderer) noexcept
{
    if (!m_uiStatusBar || !m_pTheme) return;

    // Build status text
    std::wstring statusText;
    if (!m_selectedIndices.empty())
    {
        statusText = std::to_wstring(m_selectedIndices.size()) + L" item(s) selected";
    }
    else
    {
        size_t deniedCount = 0;
        for (const auto& e : m_entries)
            if (e.accessDenied) ++deniedCount;

        statusText = std::to_wstring(m_entries.size()) + L" items";
        if (deniedCount > 0)
            statusText += L" (" + std::to_wstring(deniedCount) + L" inaccessible)";
    }

    m_uiStatusBar->SetText(statusText);

    auto uiRenderer = MakeUIRenderer(renderer);
    m_uiStatusBar->Render(uiRenderer);
}

// ============================================================================
//  Context Menu
// ============================================================================

void ExplorerWindow::ShowContextMenu(float px, float py, size_t entryIndex) noexcept
{
    if (!m_uiContextMenu) return;

    m_contextMenuTargetEntry = entryIndex;
    m_uiContextMenu->ClearItems();

    if (entryIndex < m_entries.size())
    {
        m_uiContextMenu->AddItem(L"Open", [this]() {
            if (m_contextMenuTargetEntry < m_entries.size())
            {
                const auto& entry = m_entries[m_contextMenuTargetEntry];
                if (entry.IsDirectory()) NavigateTo(entry.fullPath);
            }
        });
        m_uiContextMenu->AddSeparator();
        m_uiContextMenu->AddItem(L"Copy");
        m_uiContextMenu->AddItem(L"Cut");
        m_uiContextMenu->AddItem(L"Delete", [this]() {
            if (m_contextMenuTargetEntry < m_entries.size() && m_pFS)
            {
                const auto& entry = m_entries[m_contextMenuTargetEntry];
                if (entry.IsDirectory())
                    (void)m_pFS->EraseDirectory(entry.fullPath, true);
                else
                    (void)m_pFS->EraseFile(entry.fullPath);
                Refresh();
            }
        });
        m_uiContextMenu->AddItem(L"Rename");
        m_uiContextMenu->AddSeparator();
        m_uiContextMenu->AddItem(L"Properties");
    }
    else
    {
        m_uiContextMenu->AddItem(L"New Folder", [this]() {
            if (m_pFS)
            {
                (void)m_pFS->CreateFolder(m_pFS->Combine(m_currentPath, L"New Folder"));
                Refresh();
            }
        });
        m_uiContextMenu->AddSeparator();
        m_uiContextMenu->AddItem(L"Paste");
        m_uiContextMenu->AddSeparator();
        m_uiContextMenu->AddItem(L"Properties");
    }

    m_uiContextMenu->ShowAt(px, py);
    m_contextMenuOpen = true;
}

void ExplorerWindow::CloseContextMenu() noexcept
{
    if (m_uiContextMenu)
    {
        m_uiContextMenu->Close();
    }
    m_contextMenuOpen = false;
    m_contextMenuTargetEntry = static_cast<size_t>(-1);
}

void ExplorerWindow::RenderContextMenu(Graphics::Renderer& renderer) noexcept
{
    if (!m_uiContextMenu || !m_uiContextMenu->IsOpen() || !m_pTheme) return;

    auto uiRenderer = MakeUIRenderer(renderer);
    m_uiContextMenu->Render(uiRenderer);
}

// ============================================================================
//  Hit Testing
// ============================================================================

ExplorerHitRegion ExplorerWindow::HitTest(float px, float py) const noexcept
{
    const ExplorerLayout& lay = m_layout;

    if (lay.btnBack.Contains(px, py))       { return ExplorerHitRegion::ToolbarBack; }
    if (lay.btnForward.Contains(px, py))    { return ExplorerHitRegion::ToolbarForward; }
    if (lay.btnUp.Contains(px, py))         { return ExplorerHitRegion::ToolbarUp; }
    if (lay.btnRefresh.Contains(px, py))    { return ExplorerHitRegion::ToolbarRefresh; }
    if (lay.btnNewFolder.Contains(px, py))  { return ExplorerHitRegion::ToolbarNewFolder; }
    if (lay.btnCopy.Contains(px, py))       { return ExplorerHitRegion::ToolbarCopy; }
    if (lay.btnPaste.Contains(px, py))      { return ExplorerHitRegion::ToolbarPaste; }
    if (lay.btnDelete.Contains(px, py))     { return ExplorerHitRegion::ToolbarDelete; }
    if (lay.btnRename.Contains(px, py))     { return ExplorerHitRegion::ToolbarRename; }
    if (lay.btnProperties.Contains(px, py)) { return ExplorerHitRegion::ToolbarProperties; }
    if (lay.addressBarArea.Contains(px, py)) { return ExplorerHitRegion::AddressBar; }
    if (lay.navPaneArea.Contains(px, py))   { return ExplorerHitRegion::NavPaneItem; }
    if (lay.fileViewArea.Contains(px, py))
    {
        if (HitTestFileView(px, py) >= 0)
        {
            return ExplorerHitRegion::FileViewItem;
        }
        return ExplorerHitRegion::FileViewBackground;
    }
    if (lay.statusBarArea.Contains(px, py)) { return ExplorerHitRegion::StatusBar; }

    return ExplorerHitRegion::None;
}

int ExplorerWindow::HitTestNavPane(float px, float py) const noexcept
{
    const auto& navArea = m_layout.navPaneArea;
    if (!navArea.Contains(px, py)) { return -1; }
    const float itemH = 32.0f;

    float ey = navArea.y + 4.0f;
    for (int i = 0; i < static_cast<int>(m_navPaneEntries.size()); ++i)
    {
        const float itemBottom = ey + itemH;
        if (py >= ey && py < itemBottom) { return i; }
        ey = itemBottom;
    }

    return -1;
}

int ExplorerWindow::HitTestFileView(float px, float py) const noexcept
{
    for (int i = 0; i < static_cast<int>(m_fileViewItems.size()); ++i)
    {
        if (m_fileViewItems[i].bounds.Contains(px, py))
        {
            return i;
        }
    }
    return -1;
}

int ExplorerWindow::HitTestAddressBarSegment(float px, float py) const noexcept
{
    if (!m_layout.addressBarArea.Contains(px, py)) { return -1; }
    (void)px;
    return 0; // Simple: click on address bar = click on whole thing
}

// ============================================================================
//  Click Handlers
// ============================================================================

void ExplorerWindow::HandleToolbarClick(ExplorerHitRegion region) noexcept
{
    switch (region)
    {
    case ExplorerHitRegion::ToolbarBack:       NavigateBack();    break;
    case ExplorerHitRegion::ToolbarForward:    NavigateForward(); break;
    case ExplorerHitRegion::ToolbarUp:         NavigateUp();      break;
    case ExplorerHitRegion::ToolbarRefresh:    Refresh();         break;
    case ExplorerHitRegion::ToolbarNewFolder:
        // Placeholder: create new folder
        if (m_pFS)
        {
            (void)m_pFS->CreateFolder(m_pFS->Combine(m_currentPath, L"New Folder"));
            Refresh();
        }
        break;
    case ExplorerHitRegion::ToolbarDelete:
        if (!m_selectedIndices.empty() && m_pFS)
        {
            for (const auto& selIdx : m_selectedIndices)
            {
                if (selIdx < m_entries.size())
                {
                    const auto& entry = m_entries[selIdx];
                    if (entry.IsDirectory())
                    {
                        (void)m_pFS->EraseDirectory(entry.fullPath, true);
                    }
                    else
                    {
                        (void)m_pFS->EraseFile(entry.fullPath);
                    }
                }
            }
            ClearSelection();
            Refresh();
        }
        break;
    case ExplorerHitRegion::ToolbarCopy:
    case ExplorerHitRegion::ToolbarPaste:
    case ExplorerHitRegion::ToolbarRename:
    case ExplorerHitRegion::ToolbarProperties:
        // Placeholder actions
        break;
    default:
        break;
    }
}

void ExplorerWindow::HandleNavPaneClick(int index) noexcept
{
    if (index < 0 || index >= static_cast<int>(m_navPaneEntries.size())) { return; }
    const auto& entry = m_navPaneEntries[index];

    if (!entry.path.empty())
    {
        NavigateTo(entry.path);
    }
    else if (entry.displayName == L"This PC")
    {
        // Navigate to a virtual "This PC" - show drives
        // For now, navigate to the first drive
        if (m_pFS)
        {
            const auto drives = m_pFS->GetLogicalDrives();
            if (!drives.empty())
            {
                NavigateTo(drives[0]);
            }
        }
    }
}

void ExplorerWindow::HandleFileViewClick(int index) noexcept
{
    if (index < 0 || index >= static_cast<int>(m_fileViewItems.size())) { return; }
    const size_t entryIdx = m_fileViewItems[index].entryIndex;

    if (entryIdx >= m_entries.size()) { return; }

    if (m_pMouse->IsHeld(Input::MouseButton::Left) && (m_pMouse->IsHeld(Input::MouseButton::Right)))
    {
        // Multi-select with Ctrl or Shift (simplified: just toggle)
        SelectItem(entryIdx, true);
    }
    else
    {
        SelectItem(entryIdx, false);
    }

    CloseContextMenu();
}

void ExplorerWindow::HandleFileViewDoubleClick(int index) noexcept
{
    if (index < 0 || index >= static_cast<int>(m_fileViewItems.size())) { return; }
    const size_t entryIdx = m_fileViewItems[index].entryIndex;

    if (entryIdx >= m_entries.size()) { return; }
    const auto& entry = m_entries[entryIdx];

    if (entry.IsDirectory())
    {
        NavigateTo(entry.fullPath);
    }
    // Future: launch files with associated apps
}

void ExplorerWindow::HandleAddressBarClick(int /*index*/) noexcept
{
    // Future: enter editable path mode
}

// ============================================================================
//  Selection
// ============================================================================

void ExplorerWindow::ClearSelection() noexcept
{
    m_selectedIndices.clear();
    for (auto& item : m_fileViewItems) { item.isSelected = false; }
    m_selectAnchor = -1;
}

void ExplorerWindow::SelectItem(size_t index, bool addToSelection) noexcept
{
    if (index >= m_entries.size()) { return; }

    if (!addToSelection)
    {
        ClearSelection();
    }

    if (std::find(m_selectedIndices.begin(), m_selectedIndices.end(), index) != m_selectedIndices.end())
    {
        // Deselect
        m_selectedIndices.erase(
            std::remove(m_selectedIndices.begin(), m_selectedIndices.end(), index),
            m_selectedIndices.end());
    }
    else
    {
        m_selectedIndices.push_back(index);
    }

    m_selectAnchor = static_cast<int>(index);

    // Update item state
    for (auto& item : m_fileViewItems)
    {
        item.isSelected = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), item.entryIndex) != m_selectedIndices.end();
    }
}

void ExplorerWindow::SelectRange(size_t from, size_t to) noexcept
{
    ClearSelection();

    const size_t start = (std::min)(from, to);
    const size_t end = (std::max)(from, to);

    for (size_t i = start; i <= end && i < m_entries.size(); ++i)
    {
        m_selectedIndices.push_back(i);
    }

    for (auto& item : m_fileViewItems)
    {
        item.isSelected = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), item.entryIndex) != m_selectedIndices.end();
    }
}

// ============================================================================
//  Icon Cache
// ============================================================================

uint32_t ExplorerWindow::GetIconGlyph(const std::wstring& extension, bool isDirectory) noexcept
{
    if (isDirectory) { return 0x1F4C1; } // folder icon

    // Check cache
    for (const auto& cached : m_iconCache)
    {
        if (cached.extension == extension) { return cached.glyph; }
    }

    // Assign glyph based on extension
    uint32_t glyph = 0x1F4C4; // default file icon
    if (extension == L".txt")     { glyph = 0x1F4DD; }
    else if (extension == L".png" || extension == L".jpg" || extension == L".jpeg" || extension == L".bmp" || extension == L".gif")
    { glyph = 0x1F5BC; }
    else if (extension == L".mp3" || extension == L".wav" || extension == L".flac")
    { glyph = 0x1F3B5; }
    else if (extension == L".mp4" || extension == L".avi" || extension == L".mkv")
    { glyph = 0x1F3AC; }
    else if (extension == L".pdf") { glyph = 0x1F4D5; }
    else if (extension == L".zip" || extension == L".rar" || extension == L".7z")
    { glyph = 0x1F4E6; }
    else if (extension == L".exe" || extension == L".dll")
    { glyph = 0x2699; }
    else if (extension == L".doc" || extension == L".docx")
    { glyph = 0x1F4DD; }
    else if (extension == L".xls" || extension == L".xlsx")
    { glyph = 0x1F4CA; }
    else if (extension == L".ppt" || extension == L".pptx")
    { glyph = 0x1F4CA; }

    // Add to cache
    m_iconCache.push_back({ extension, glyph });
    return glyph;
}

// ============================================================================
//  Accessors
// ============================================================================

uint64_t ExplorerWindow::GetWindowId() const noexcept
{
    return m_pWindow ? m_pWindow->GetId() : 0;
}

} // namespace DragonOS::Explorer
