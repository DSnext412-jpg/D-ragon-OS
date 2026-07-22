#include <StartMenu/StartMenuController.hpp>
#include <StartMenu/StartMenuAnimations.hpp>

#include <Animation/AnimationManager.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Notifications/NotificationManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Theme/ThemePalette.hpp>
#include <Apps/ApplicationRegistry.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace DragonOS::StartMenu {

// ============================================================================
//  Construction / Destruction
// ============================================================================

StartMenuController::StartMenuController() noexcept = default;

StartMenuController::~StartMenuController() noexcept
{
    Shutdown();
}

// ============================================================================
//  Lifecycle
// ============================================================================

bool StartMenuController::Initialize(
    Theme::ThemeManager&         themeManager,
    Input::MouseManager&         mouseManager,
    Animation::AnimationManager& animManager,
    Apps::ApplicationRegistry&   appRegistry) noexcept
{
    if (m_initialized) { return true; }

    m_pThemeManager = &themeManager;
    m_pMouse        = &mouseManager;
    m_pAnimManager  = &animManager;
    m_pAppRegistry  = &appRegistry;

    m_initialized = true;
    return true;
}

void StartMenuController::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_pinnedTiles.clear();
    m_allAppsEntries.clear();
    m_pThemeManager = nullptr;
    m_pMouse        = nullptr;
    m_pAnimManager  = nullptr;
    m_pAppRegistry  = nullptr;
    m_initialized   = false;
}

// ============================================================================
//  Open / Close / Toggle
// ============================================================================

void StartMenuController::Open() noexcept
{
    if (!m_initialized) { return; }
    if (m_isOpen) { return; }

    m_isOpen = true;
    m_allAppsBuilt = false;
    m_focusedItemIdx = -1;

    StartOpenAnimation();
}

void StartMenuController::Close() noexcept
{
    if (!m_initialized) { return; }
    if (!m_isOpen) { return; }

    m_isOpen = false;
    StartCloseAnimation();
}

void StartMenuController::Toggle() noexcept
{
    if (m_isOpen) { Close(); }
    else          { Open(); }
}

// ============================================================================
//  Layout
// ============================================================================

StartMenuController::Layout StartMenuController::CalculateLayout(
    float vpW, float vpH) const noexcept
{
    Layout lay{};

    const float menuW = (std::min)(600.0f, vpW - 8.0f);
    const float taskbarH = Theme::ThemeMetrics::TaskbarHeight;
    const float menuMaxH = vpH - taskbarH - 8.0f;
    const float menuH = (std::min)(720.0f, (std::max)(480.0f, menuMaxH));

    const float menuX = 4.0f;
    const float menuY = vpH - taskbarH - menuH - 4.0f;

    lay.menu = { menuX, menuY, menuW, menuH };

    // ── Header (user avatar, name, date) ────────────────────────────────
    const float headerH = 72.0f;
    lay.headerArea = { menuX, menuY, menuW, headerH };

    // ── Search box ──────────────────────────────────────────────────────
    const float searchY = menuY + headerH + 8.0f;
    const float searchH = 36.0f;
    lay.searchArea = { menuX + 12.0f, searchY, menuW - 24.0f, searchH };

    // ── Pinned apps grid ────────────────────────────────────────────────
    const float pinnedY = searchY + searchH + 12.0f;
    const float gridItemSize = 64.0f;
    const float gridGap = 8.0f;
    const int   cols = 6;
    const float gridW = cols * (gridItemSize + gridGap) - gridGap;
    const float gridX = menuX + (menuW - gridW) * 0.5f;
    const int   pinnedRows = 2;
    const float pinnedH = pinnedRows * (gridItemSize + gridGap) - gridGap;
    lay.pinnedArea = { gridX, pinnedY, gridW, pinnedH };

    // ── All apps list ───────────────────────────────────────────────────
    const float allAppsY = pinnedY + pinnedH + 16.0f;
    const float allAppsH = menuY + menuH - allAppsY - 48.0f;
    lay.allAppsArea = { menuX + 4.0f, allAppsY, menuW - 8.0f, allAppsH };

    // ── Power section (bottom-right) ────────────────────────────────────
    const float powerBtnSize = 36.0f;
    const float powerGap = 4.0f;
    const float powerY = menuY + menuH - powerBtnSize - 8.0f;
    const float powerClusterW = 5.0f * (powerBtnSize + powerGap) - powerGap;

    float px = menuX + menuW - powerClusterW - 8.0f;
    const float py = powerY;
    lay.powerShutdown = { px, py, powerBtnSize, powerBtnSize }; px += powerBtnSize + powerGap;
    lay.powerRestart  = { px, py, powerBtnSize, powerBtnSize }; px += powerBtnSize + powerGap;
    lay.powerSleep    = { px, py, powerBtnSize, powerBtnSize }; px += powerBtnSize + powerGap;
    lay.powerLock     = { px, py, powerBtnSize, powerBtnSize }; px += powerBtnSize + powerGap;
    lay.powerSignOut  = { px, py, powerBtnSize, powerBtnSize };
    lay.powerArea     = { menuX + menuW - powerClusterW - 8.0f, powerY, powerClusterW, powerBtnSize };

    // ── Settings shortcut (bottom-left) ─────────────────────────────────
    const float settingsW = 100.0f;
    lay.settingsArea = { menuX + 8.0f, powerY, settingsW, powerBtnSize };

    return lay;
}

void StartMenuController::BuildAppEntries() noexcept
{
    if (!m_pAppRegistry) { return; }

    // ── Pinned tiles ────────────────────────────────────────────────────
    m_pinnedTiles.clear();

    const auto pinned = m_pAppRegistry->GetPinned();
    const float gridItemSize = 64.0f;
    const float gridGap = 8.0f;
    const int cols = 6;

    for (size_t i = 0; i < pinned.size(); ++i)
    {
        const int row = static_cast<int>(i) / cols;
        const int col = static_cast<int>(i) % cols;

        PinnedAppTile tile;
        tile.pApp = pinned[i];
        tile.bounds = {
            m_layout.pinnedArea.x + col * (gridItemSize + gridGap),
            m_layout.pinnedArea.y + row * (gridItemSize + gridGap),
            gridItemSize,
            gridItemSize
        };
        tile.isHovered = false;
        tile.isPressed = false;
        m_pinnedTiles.push_back(std::move(tile));
    }

    // ── All apps entries ────────────────────────────────────────────────
    m_allAppsEntries.clear();

    const auto allApps = m_pAppRegistry->GetAll();
    const float itemH = 36.0f;
    const float itemX = m_layout.allAppsArea.x;
    const float itemW = m_layout.allAppsArea.width;

    float cy = m_layout.allAppsArea.y;
    std::wstring currentSection;

    for (const auto* app : allApps)
    {
        // Section header (first letter / category)
        std::wstring sectionLabel;
        if (app->pinned)
        {
            sectionLabel = L"Pinned";
        }
        else
        {
            wchar_t firstChar = towupper(app->displayName[0]);
            sectionLabel = std::wstring(1, firstChar);
        }

        if (sectionLabel != currentSection)
        {
            AllAppEntry sectionEntry;
            sectionEntry.pApp = nullptr;
            sectionEntry.sectionLabel = sectionLabel;
            sectionEntry.bounds = { itemX, cy, itemW, 24.0f };
            sectionEntry.isHovered = false;
            m_allAppsEntries.push_back(std::move(sectionEntry));
            cy += 24.0f;
            currentSection = sectionLabel;
        }

        AllAppEntry entry;
        entry.pApp = app;
        entry.bounds = { itemX + 8.0f, cy, itemW - 8.0f, itemH };
        cy += itemH;

        m_allAppsEntries.push_back(std::move(entry));
    }

    m_layout.allAppsContentHeight = cy - m_layout.allAppsArea.y;
    m_allAppsBuilt = true;
}

// ============================================================================
//  Per-frame
// ============================================================================

void StartMenuController::Resize(float viewportWidth, float viewportHeight) noexcept
{
    if (!m_initialized) { return; }

    m_viewportWidth  = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_layout = CalculateLayout(viewportWidth, viewportHeight);
    m_bounds = m_layout.menu;

    if (m_allAppsBuilt)
    {
        BuildAppEntries();
    }
}

void StartMenuController::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }
    if (!m_isOpen && m_animProgress < 0.001f) { return; }

    // ── Update header date ──────────────────────────────────────────────
    // RenderHeader reads system time directly each frame

    // ── Update animations ───────────────────────────────────────────────
    UpdateAnimations(deltaTime);
}

void StartMenuController::UpdateAnimations(float deltaTime) noexcept
{
    // Spring-physics style velocity-driven animation for smooth open/close
    const float springStiffness = 12.0f;
    const float damping = 6.0f;

    const float diff = m_animTarget - m_animProgress;
    m_animVelocity += diff * springStiffness * deltaTime;
    m_animVelocity *= (std::max)(0.0f, 1.0f - damping * deltaTime);
    m_animProgress += m_animVelocity * deltaTime;
    m_animProgress = (std::max)(0.0f, (std::min)(1.0f, m_animProgress));
}

void StartMenuController::StartOpenAnimation() noexcept
{
    m_animTarget = 1.0f;
    m_animVelocity = 0.0f;
    if (m_animProgress < 0.001f)
    {
        m_animProgress = 0.001f; // Kick-start
    }
}

void StartMenuController::StartCloseAnimation() noexcept
{
    m_animTarget = 0.0f;
    m_animVelocity = 0.0f;
}

// ============================================================================
//  Input
// ============================================================================

void StartMenuController::ProcessInput() noexcept
{
    if (!m_initialized || !m_pMouse) { return; }
    if (!m_isOpen && m_animProgress < 0.001f) { return; }

    const auto pos = m_pMouse->GetPosition();
    const auto& menu = m_layout.menu;

    // Reset per-frame hover/pressed state for items
    for (auto& tile : m_pinnedTiles) { tile.isHovered = false; }
    for (auto& entry : m_allAppsEntries) { entry.isHovered = false; }
    m_powerShutdown.isHovered = false;
    m_powerRestart.isHovered = false;
    m_powerSleep.isHovered = false;
    m_powerLock.isHovered = false;
    m_powerSignOut.isHovered = false;
    m_settingsButton.isHovered = false;

    m_hoveredRegion = StartMenuHitRegion::None;
    m_hoveredPinnedIdx = -1;
    m_hoveredAllAppsIdx = -1;

    // If click outside menu bounds -> close
    if (m_pMouse->WasLeftClicked() && !menu.Contains(pos.x, pos.y))
    {
        // Check if we're inside the taskbar start button (handled by taskbar)
        // For any other outside click, close the menu
        const float taskbarBottom = m_viewportHeight;
        const float taskbarTop = m_viewportHeight - Theme::ThemeMetrics::TaskbarHeight;
        const bool inTaskbar = (pos.y >= taskbarTop && pos.y <= taskbarBottom);

        if (!inTaskbar)
        {
            Close();
            return;
        }
    }

    if (!menu.Contains(pos.x, pos.y))
    {
        return;
    }

    m_hoveredRegion = HitTestMenu(pos.x, pos.y);

    // ── Handle clicks ────────────────────────────────────────────────────
    if (m_pMouse->WasLeftClicked())
    {
        m_pressedRegion = m_hoveredRegion;

        switch (m_hoveredRegion)
        {
        case StartMenuHitRegion::PinnedApp:
        {
            const int idx = HitTestPinnedIndex(pos.x, pos.y);
            if (idx >= 0 && idx < static_cast<int>(m_pinnedTiles.size()))
            {
                m_pinnedTiles[idx].isPressed = true;
                if (m_launchCallback && m_pinnedTiles[idx].pApp)
                {
                    m_launchCallback(m_pinnedTiles[idx].pApp);
                }
                Close();
            }
            break;
        }

        case StartMenuHitRegion::AllAppItem:
        {
            const int idx = HitTestAllAppsIndex(pos.x, pos.y);
            if (idx >= 0 && idx < static_cast<int>(m_allAppsEntries.size()))
            {
                if (m_allAppsEntries[idx].pApp)
                {
                    m_allAppsEntries[idx].isPressed = true;
                    if (m_launchCallback)
                    {
                        m_launchCallback(m_allAppsEntries[idx].pApp);
                    }
                    Close();
                }
            }
            break;
        }

        case StartMenuHitRegion::PowerShutdown:
            m_powerShutdown.isPressed = true;
            Close();
            break;

        case StartMenuHitRegion::PowerRestart:
            m_powerRestart.isPressed = true;
            Close();
            break;

        case StartMenuHitRegion::PowerSleep:
            m_powerSleep.isPressed = true;
            Close();
            break;

        case StartMenuHitRegion::PowerLock:
            m_powerLock.isPressed = true;
            Close();
            break;

        case StartMenuHitRegion::PowerSignOut:
            m_powerSignOut.isPressed = true;
            Close();
            break;

        case StartMenuHitRegion::SettingsShortcut:
            m_settingsButton.isPressed = true;
            // Future: raise settings event
            Close();
            break;

        default:
            break;
        }
    }

    // ── Handle release ──────────────────────────────────────────────────
    if (m_pMouse->WasReleased(Input::MouseButton::Left))
    {
        for (auto& tile : m_pinnedTiles) { tile.isPressed = false; }
        for (auto& entry : m_allAppsEntries) { entry.isPressed = false; }
        m_powerShutdown.isPressed = false;
        m_powerRestart.isPressed = false;
        m_powerSleep.isPressed = false;
        m_powerLock.isPressed = false;
        m_powerSignOut.isPressed = false;
        m_settingsButton.isPressed = false;
        m_pressedRegion = StartMenuHitRegion::None;
    }

    // ── Update individual hover states ──────────────────────────────────
    if (m_hoveredRegion == StartMenuHitRegion::PinnedApp)
    {
        m_hoveredPinnedIdx = HitTestPinnedIndex(pos.x, pos.y);
        if (m_hoveredPinnedIdx >= 0 &&
            m_hoveredPinnedIdx < static_cast<int>(m_pinnedTiles.size()))
        {
            m_pinnedTiles[m_hoveredPinnedIdx].isHovered = true;
        }
    }
    else if (m_hoveredRegion == StartMenuHitRegion::AllAppItem)
    {
        m_hoveredAllAppsIdx = HitTestAllAppsIndex(pos.x, pos.y);
        if (m_hoveredAllAppsIdx >= 0 &&
            m_hoveredAllAppsIdx < static_cast<int>(m_allAppsEntries.size()))
        {
            if (m_allAppsEntries[m_hoveredAllAppsIdx].pApp)
            {
                m_allAppsEntries[m_hoveredAllAppsIdx].isHovered = true;
            }
        }
    }
    else if (m_hoveredRegion == StartMenuHitRegion::PowerShutdown)
    {
        m_powerShutdown.isHovered = true;
    }
    else if (m_hoveredRegion == StartMenuHitRegion::PowerRestart)
    {
        m_powerRestart.isHovered = true;
    }
    else if (m_hoveredRegion == StartMenuHitRegion::PowerSleep)
    {
        m_powerSleep.isHovered = true;
    }
    else if (m_hoveredRegion == StartMenuHitRegion::PowerLock)
    {
        m_powerLock.isHovered = true;
    }
    else if (m_hoveredRegion == StartMenuHitRegion::PowerSignOut)
    {
        m_powerSignOut.isHovered = true;
    }
    else if (m_hoveredRegion == StartMenuHitRegion::SettingsShortcut)
    {
        m_settingsButton.isHovered = true;
    }
}

// ============================================================================
//  Hit testing
// ============================================================================

StartMenuHitRegion StartMenuController::HitTestMenu(float px, float py) const noexcept
{
    if (m_layout.searchArea.Contains(px, py))       return StartMenuHitRegion::SearchBox;
    if (HitTestPinnedIndex(px, py) >= 0)             return StartMenuHitRegion::PinnedApp;
    if (HitTestAllAppsIndex(px, py) >= 0)            return StartMenuHitRegion::AllAppItem;
    if (m_layout.powerShutdown.Contains(px, py))     return StartMenuHitRegion::PowerShutdown;
    if (m_layout.powerRestart.Contains(px, py))      return StartMenuHitRegion::PowerRestart;
    if (m_layout.powerSleep.Contains(px, py))        return StartMenuHitRegion::PowerSleep;
    if (m_layout.powerLock.Contains(px, py))         return StartMenuHitRegion::PowerLock;
    if (m_layout.powerSignOut.Contains(px, py))      return StartMenuHitRegion::PowerSignOut;
    if (m_layout.settingsArea.Contains(px, py))      return StartMenuHitRegion::SettingsShortcut;
    if (m_layout.headerArea.Contains(px, py))        return StartMenuHitRegion::Header;
    return StartMenuHitRegion::Background;
}

int StartMenuController::HitTestPinnedIndex(float px, float py) const noexcept
{
    for (int i = 0; i < static_cast<int>(m_pinnedTiles.size()); ++i)
    {
        if (m_pinnedTiles[i].bounds.Contains(px, py))
        {
            return i;
        }
    }
    return -1;
}

int StartMenuController::HitTestAllAppsIndex(float px, float py) const noexcept
{
    for (int i = 0; i < static_cast<int>(m_allAppsEntries.size()); ++i)
    {
        if (m_allAppsEntries[i].bounds.Contains(px, py) &&
            m_allAppsEntries[i].pApp != nullptr)
        {
            return i;
        }
    }
    return -1;
}

// ============================================================================
//  Rendering
// ============================================================================

void StartMenuController::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized) { return; }
    if (!m_isOpen && m_animProgress < 0.001f) { return; }

    // Animation progress controls fade/scale of all elements
    // TODO: Apply scale via render target transform in future

    RenderBackground(renderer);
    RenderHeader(renderer);
    RenderSearchBox(renderer);
    RenderPinnedGrid(renderer);
    RenderAllAppsList(renderer);
    RenderPowerSection(renderer);
    RenderSettingsShortcut(renderer);
}

void StartMenuController::RenderBackground(Graphics::Renderer& renderer) noexcept
{
    const auto& bgColor = m_pThemeManager->GetColor(Theme::SemanticColor::StartMenuBackground);
    const Graphics::Color bg{ bgColor.r, bgColor.g, bgColor.b, bgColor.a * m_animProgress };

    const auto& m = m_layout.menu;
    const D2D1_RECT_F rect = D2D1::RectF(m.x, m.y, m.Right(), m.Bottom());

    renderer.FillRectangle(rect, bg);

    // Border
    const auto& borderColor = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color bc{ borderColor.r, borderColor.g, borderColor.b, borderColor.a * m_animProgress };
    renderer.DrawRectangle(rect, bc, 1.0f);
}

void StartMenuController::RenderHeader(Graphics::Renderer& renderer) noexcept
{
    const auto& h = m_layout.headerArea;

    // ── Greeting ────────────────────────────────────────────────────────
    const auto now = std::chrono::system_clock::now();
    const auto t   = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
    ::localtime_s(&local, &t);

    const int hour = local.tm_hour;
    std::wstring greeting = (hour < 12) ? L"Good morning" :
                            (hour < 17) ? L"Good afternoon" :
                                          L"Good evening";

    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a * m_animProgress };

    // Greeting text
    const D2D1_RECT_F greetRect = D2D1::RectF(
        h.x + 16.0f, h.y + 8.0f,
        h.Right() - 16.0f, h.y + 32.0f);
    renderer.DrawText(greeting, greetRect, tc);

    // Date
    std::wostringstream dateSS;
    dateSS << std::put_time(&local, L"%A, %B %d, %Y");
    const std::wstring dateStr = dateSS.str();

    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color sc{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };
    const D2D1_RECT_F dateRect = D2D1::RectF(
        h.x + 16.0f, h.y + 34.0f,
        h.Right() - 16.0f, h.y + 56.0f);
    renderer.DrawText(dateStr, dateRect, sc);

    // ── Notification badge ──────────────────────────────────────────────
    if (m_pNotifMgr && m_pNotifMgr->HasUnread())
    {
        const auto& accCol = m_pThemeManager->GetColor(Theme::SemanticColor::Accent);
        const Graphics::Color badgeBg{ accCol.r, accCol.g, accCol.b, accCol.a * m_animProgress };

        const std::wstring countStr = std::to_wstring(m_pNotifMgr->GetUnreadCount());
        const float badgeW = 20.0f;
        const float badgeH = 20.0f;
        const D2D1_RECT_F badgeRect = D2D1::RectF(
            h.Right() - badgeW - 12.0f, h.y + 8.0f,
            h.Right() - 12.0f, h.y + 8.0f + badgeH);
        renderer.FillRectangle(badgeRect, badgeBg);

        const Graphics::Color badgeText{ 1.0f, 1.0f, 1.0f, 1.0f * m_animProgress };
        const D2D1_RECT_F badgeTextRect = D2D1::RectF(
            badgeRect.left, badgeRect.top,
            badgeRect.right, badgeRect.bottom);
        renderer.DrawText(countStr, badgeTextRect, badgeText);
    }
}

void StartMenuController::RenderSearchBox(Graphics::Renderer& renderer) noexcept
{
    const auto& sb = m_layout.searchArea;
    const D2D1_RECT_F sbRect = D2D1::RectF(sb.x, sb.y, sb.Right(), sb.Bottom());

    // Background
    const auto& bgColor = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBackground);
    const Graphics::Color bg{ bgColor.r, bgColor.g, bgColor.b, bgColor.a * m_animProgress };
    renderer.FillRectangle(sbRect, bg);

    // Border
    const auto& borderColor = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color bc{ borderColor.r, borderColor.g, borderColor.b, borderColor.a * m_animProgress };
    renderer.DrawRectangle(sbRect, bc, 1.0f);

    // Placeholder text
    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color phCol{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };
    const D2D1_RECT_F phRect = D2D1::RectF(
        sb.x + 8.0f, sb.y + 4.0f,
        sb.Right() - 8.0f, sb.Bottom() - 4.0f);
    renderer.DrawText(L"Search apps, settings and more...", phRect, phCol);
}

void StartMenuController::RenderPinnedGrid(Graphics::Renderer& renderer) noexcept
{
    // Section title
    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color titleCol{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };

    const D2D1_RECT_F titleRect = D2D1::RectF(
        m_layout.pinnedArea.x, m_layout.pinnedArea.y - 20.0f,
        m_layout.pinnedArea.Right(), m_layout.pinnedArea.y - 4.0f);
    renderer.DrawText(L"Pinned", titleRect, titleCol);

    // Divider line
    const auto& borderCol = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color divCol{ borderCol.r, borderCol.g, borderCol.b, 0.3f * m_animProgress };
    renderer.DrawLine(
        D2D1::Point2F(m_layout.pinnedArea.x, m_layout.pinnedArea.y - 2.0f),
        D2D1::Point2F(m_layout.pinnedArea.Right(), m_layout.pinnedArea.y - 2.0f),
        divCol, 1.0f);

    // Render each tile
    for (const auto& tile : m_pinnedTiles)
    {
        RenderPinnedTile(renderer, tile);
    }
}

void StartMenuController::RenderPinnedTile(
    Graphics::Renderer& renderer,
    const PinnedAppTile& tile) noexcept
{
    if (!tile.pApp) { return; }

    const auto& b = tile.bounds;
    const D2D1_RECT_F tileRect = D2D1::RectF(b.x, b.y, b.Right(), b.Bottom());

    // Hover background
    if (tile.isHovered || tile.hoverAnim > 0.001f)
    {
        const auto& hoverColor = m_pThemeManager->GetColor(Theme::SemanticColor::StartMenuItemHover);
        const Graphics::Color hc{ hoverColor.r, hoverColor.g, hoverColor.b,
                                   hoverColor.a * m_animProgress };
        renderer.FillRectangle(tileRect, hc);
    }

    // Placeholder icon (first letter of display name)
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a * m_animProgress };

    const wchar_t iconChar = tile.pApp->displayName[0];
    const D2D1_RECT_F iconRect = D2D1::RectF(
        b.x + 4.0f, b.y + 4.0f,
        b.Right() - 4.0f, b.y + b.height * 0.55f);
    renderer.DrawText(std::wstring_view(&iconChar, 1), iconRect, tc);

    // Label below icon
    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color labelCol{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };
    const D2D1_RECT_F labelRect = D2D1::RectF(
        b.x + 2.0f, b.y + b.height * 0.55f,
        b.Right() - 2.0f, b.Bottom() - 2.0f);
    renderer.DrawText(tile.pApp->displayName, labelRect, labelCol);
}

void StartMenuController::RenderAllAppsList(Graphics::Renderer& renderer) noexcept
{
    if (!m_allAppsBuilt)
    {
        BuildAppEntries();
    }

    // Section title
    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color titleCol{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };

    const D2D1_RECT_F titleRect = D2D1::RectF(
        m_layout.allAppsArea.x, m_layout.allAppsArea.y - 16.0f,
        m_layout.allAppsArea.Right(), m_layout.allAppsArea.y - 2.0f);
    renderer.DrawText(L"All apps", titleRect, titleCol);

    // Divider
    const auto& borderCol = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color divCol{ borderCol.r, borderCol.g, borderCol.b, 0.3f * m_animProgress };
    renderer.DrawLine(
        D2D1::Point2F(m_layout.allAppsArea.x, m_layout.allAppsArea.y - 2.0f),
        D2D1::Point2F(m_layout.allAppsArea.Right(), m_layout.allAppsArea.y - 2.0f),
        divCol, 1.0f);

    // Render visible entries
    for (const auto& entry : m_allAppsEntries)
    {
        RenderAllAppEntry(renderer, entry);
    }
}

void StartMenuController::RenderAllAppEntry(
    Graphics::Renderer& renderer,
    const AllAppEntry& entry) noexcept
{
    const auto& b = entry.bounds;
    const D2D1_RECT_F entryRect = D2D1::RectF(b.x, b.y, b.Right(), b.Bottom());

    if (!entry.pApp)
    {
        // Section label
        const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
        const Graphics::Color sc{ secCol.r, secCol.g, secCol.b, secCol.a * m_animProgress };
        renderer.DrawText(entry.sectionLabel, entryRect, sc);
        return;
    }

    // Hover background
    if (entry.isHovered || entry.hoverAnim > 0.001f)
    {
        const auto& hoverColor = m_pThemeManager->GetColor(Theme::SemanticColor::StartMenuItemHover);
        const Graphics::Color hc{ hoverColor.r, hoverColor.g, hoverColor.b,
                                   hoverColor.a * m_animProgress };
        renderer.FillRectangle(entryRect, hc);
    }

    // Name
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a * m_animProgress };

    const D2D1_RECT_F nameRect = D2D1::RectF(
        b.x + 8.0f, b.y + 2.0f,
        b.Right() - 4.0f, b.Bottom() - 2.0f);
    renderer.DrawText(entry.pApp->displayName, nameRect, tc);
}

void StartMenuController::RenderPowerSection(Graphics::Renderer& renderer) noexcept
{
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a * m_animProgress };

    const auto renderPowerBtn = [&](const PowerButton& btn, const wchar_t* label)
    {
        const D2D1_RECT_F r = D2D1::RectF(
            btn.bounds.x, btn.bounds.y,
            btn.bounds.Right(), btn.bounds.Bottom());

        if (btn.isHovered)
        {
            const auto& hoverColor = m_pThemeManager->GetColor(Theme::SemanticColor::StartMenuItemHover);
            const Graphics::Color hc{ hoverColor.r, hoverColor.g, hoverColor.b,
                                       hoverColor.a * m_animProgress };
            renderer.FillRectangle(r, hc);
        }

        renderer.DrawText(label, r, tc);
    };

    renderPowerBtn(m_powerShutdown, L"Shut");
    renderPowerBtn(m_powerRestart,  L"Rest");
    renderPowerBtn(m_powerSleep,    L"Slp");
    renderPowerBtn(m_powerLock,     L"Lock");
    renderPowerBtn(m_powerSignOut,  L"Sign");
}

void StartMenuController::RenderSettingsShortcut(Graphics::Renderer& renderer) noexcept
{
    const auto& sb = m_layout.settingsArea;
    const D2D1_RECT_F r = D2D1::RectF(sb.x, sb.y, sb.Right(), sb.Bottom());

    if (m_settingsButton.isHovered)
    {
        const auto& hoverColor = m_pThemeManager->GetColor(Theme::SemanticColor::StartMenuItemHover);
        const Graphics::Color hc{ hoverColor.r, hoverColor.g, hoverColor.b,
                                   hoverColor.a * m_animProgress };
        renderer.FillRectangle(r, hc);
    }

    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a * m_animProgress };

    const D2D1_RECT_F labelRect = D2D1::RectF(
        sb.x + 4.0f, sb.y + 4.0f,
        sb.Right() - 4.0f, sb.Bottom() - 4.0f);
    renderer.DrawText(L"Settings", labelRect, tc);
}

} // namespace DragonOS::StartMenu
