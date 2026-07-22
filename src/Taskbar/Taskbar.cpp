#include <Taskbar/Taskbar.hpp>

#include <Animation/AnimationManager.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Notifications/NotificationManager.hpp>
#include <Services/ServiceManager.hpp>
#include <StartMenu/StartMenuController.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <Theme/ThemePalette.hpp>
#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/WindowCollection.hpp>

#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace DragonOS::Taskbar {

// ============================================================================
//  Construction / Destruction
// ============================================================================

Taskbar::Taskbar() noexcept = default;

Taskbar::~Taskbar() noexcept
{
    Shutdown();
}

// ============================================================================
//  Lifecycle
// ============================================================================

bool Taskbar::Initialize(
    Theme::ThemeManager&       themeManager,
    Input::MouseManager&       mouseManager,
    WindowManager::WindowManager& windowManager,
    Animation::AnimationManager&  animManager) noexcept
{
    if (m_initialized) { return true; }

    m_pThemeManager   = &themeManager;
    m_pMouse          = &mouseManager;
    m_pWindowManager  = &windowManager;
    m_pAnimManager    = &animManager;

    m_initialized = true;
    return true;
}

void Taskbar::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_taskItems.clear();
    m_pThemeManager  = nullptr;
    m_pMouse         = nullptr;
    m_pWindowManager = nullptr;
    m_pAnimManager   = nullptr;
    m_initialized    = false;
}

// ============================================================================
//  Layout
// ============================================================================

Taskbar::Layout Taskbar::CalculateLayout(
    float vpW, float vpH, float height) const noexcept
{
    Layout lay{};

    const float barX = 0.0f;
    const float barY = vpH - height;
    lay.bar = { barX, barY, vpW, height };

    // ── Start button ─────────────────────────────────────────────────────
    const float sbSize  = 36.0f;
    const float sbPad   = 6.0f;
    const float sbX = barX + sbPad;
    const float sbY = barY + (height - sbSize) * 0.5f;
    lay.startButton = { sbX, sbY, sbSize, sbSize };

    // ── Search button ────────────────────────────────────────────────────
    const float searchSize = 36.0f;
    const float searchX = sbX + sbSize + sbPad;
    const float searchY = barY + (height - searchSize) * 0.5f;
    lay.searchButton = { searchX, searchY, searchSize, searchSize };

    // ── Clock (right-aligned) ────────────────────────────────────────────
    const float clockW = 120.0f;
    const float clockH = height;
    const float clockX = vpW - clockW;
    lay.clockArea = { clockX, barY, clockW, clockH };

    // ── System tray (between task list and clock) ────────────────────────
    const float trayIconSize = 24.0f;
    const float trayPad      = 4.0f;
    const float traySpacing  = 2.0f;
    const int trayCount = 4;
    const float trayTotalW = trayCount * (trayIconSize + traySpacing) - traySpacing + trayPad * 2.0f;
    const float trayX = clockX - trayTotalW;
    const float trayY = barY + (height - trayIconSize) * 0.5f;

    lay.trayArea = { trayX, barY, trayTotalW, height };

    float tx = trayX + trayPad;
    lay.trayVolume        = { tx, trayY, trayIconSize, trayIconSize }; tx += trayIconSize + traySpacing;
    lay.trayNetwork       = { tx, trayY, trayIconSize, trayIconSize }; tx += trayIconSize + traySpacing;
    lay.trayBattery       = { tx, trayY, trayIconSize, trayIconSize }; tx += trayIconSize + traySpacing;
    lay.trayNotifications = { tx, trayY, trayIconSize, trayIconSize };

    // ── Activity indicator (next to notification tray) ───────────────────
    const float actSize = 8.0f;
    lay.activityIndicator = { tx + trayIconSize + traySpacing, trayY + (trayIconSize - actSize) * 0.5f, actSize, actSize };

    // ── Task list (between search button and tray) ───────────────────────
    const float searchRight = searchX + searchSize + sbPad;
    lay.taskListArea = { searchRight, barY, trayX - searchRight, height };

    return lay;
}

// ============================================================================
//  Per-frame
// ============================================================================

void Taskbar::Resize(float viewportWidth, float viewportHeight) noexcept
{
    if (!m_initialized) { return; }

    m_viewportWidth  = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_layout = CalculateLayout(viewportWidth, viewportHeight, m_height);
}

void Taskbar::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    // ── Sync task list with WindowManager ────────────────────────────────
    m_taskItems.clear();

    const auto& windows = m_pWindowManager->GetCollection().GetAll();
    const auto* focused = m_pWindowManager->GetFocusedWindow();
    m_lastFocusedId = focused ? focused->GetId() : 0;

    for (const auto& w : windows)
    {
        if (!w->IsVisible() || w->GetState() == WindowManager::WindowState::Hidden)
        {
            continue;
        }

        TaskItem item;
        item.windowId = w->GetId();
        item.title    = w->GetTitle();
        item.isActive = (w->GetId() == m_lastFocusedId);
        item.isHovered = false;
        item.isPressed = false;
        m_taskItems.push_back(std::move(item));
    }

    // ── Compute task item bounds ─────────────────────────────────────────
    const float itemSpacing = 4.0f;
    const float itemXStart  = m_layout.taskListArea.x + itemSpacing;
    const float itemY       = m_layout.bar.y + 4.0f;
    const float itemH       = m_height - 8.0f;

    float cx = itemXStart;
    for (auto& ti : m_taskItems)
    {
        const float textW = static_cast<float>(ti.title.size()) * 7.2f;
        const float itemW = (std::max)(80.0f, (std::min)(textW + 24.0f, 200.0f));
        ti.bounds = { cx, itemY, itemW, itemH };
        cx += itemW + itemSpacing;
    }

    // ── Update animations ────────────────────────────────────────────────
    UpdateAnimations(deltaTime);

    // ── Update clock ─────────────────────────────────────────────────────
    m_clockTimer += deltaTime;
    if (m_clockTimer >= 1.0f || m_timeString.empty())
    {
        m_clockTimer = 0.0f;

        const auto now = std::chrono::system_clock::now();
        const auto t   = std::chrono::system_clock::to_time_t(now);

        std::tm local{};
        ::localtime_s(&local, &t);

        std::wostringstream timeSS;
        timeSS << std::put_time(&local, L"%I:%M %p");
        m_timeString = timeSS.str();

        std::wostringstream dateSS;
        dateSS << std::put_time(&local, L"%m/%d/%Y");
        m_dateString = dateSS.str();
    }
}

void Taskbar::UpdateAnimations(float deltaTime) noexcept
{
    const float speed = 8.0f;

    // Start button hover animation
    if (m_hoveredRegion == TaskbarHitRegion::StartButton ||
        m_pressedRegion == TaskbarHitRegion::StartButton)
    {
        m_startBtnHoverAnim = (std::min)(1.0f, m_startBtnHoverAnim + speed * deltaTime);
    }
    else
    {
        m_startBtnHoverAnim = (std::max)(0.0f, m_startBtnHoverAnim - speed * deltaTime);
    }

    // Task item hover animations
    for (auto& ti : m_taskItems)
    {
        if (ti.isHovered)
        {
            ti.hoverAnim = (std::min)(1.0f, ti.hoverAnim + speed * deltaTime);
        }
        else
        {
            ti.hoverAnim = (std::max)(0.0f, ti.hoverAnim - speed * deltaTime);
        }
    }
}

void Taskbar::ProcessInput() noexcept
{
    if (!m_initialized || !m_pMouse) { return; }

    const auto pos = m_pMouse->GetPosition();
    const auto barBounds = GetBounds();

    // Reset hover state
    TaskbarHitRegion newHover = TaskbarHitRegion::None;

    if (barBounds.Contains(pos.x, pos.y))
    {
        // ── Check start button ──────────────────────────────────────────
        if (m_layout.startButton.Contains(pos.x, pos.y))
        {
            newHover = TaskbarHitRegion::StartButton;
        }
        // ── Check search button ─────────────────────────────────────────
        else if (m_layout.searchButton.Contains(pos.x, pos.y))
        {
            newHover = TaskbarHitRegion::SearchButton;
        }
        // ── Check task items (reverse: rightmost first) ──────────────────
        else
        {
            bool hitTask = false;
            for (auto it = m_taskItems.rbegin(); it != m_taskItems.rend(); ++it)
            {
                if (it->bounds.Contains(pos.x, pos.y))
                {
                    newHover = TaskbarHitRegion::TaskItem;
                    it->isHovered = true;
                    hitTask = true;
                    break;
                }
            }

            if (!hitTask)
            {
                for (auto& ti : m_taskItems) { ti.isHovered = false; }

                // ── Check tray icons ───────────────────────────────────
                if (m_layout.trayVolume.Contains(pos.x, pos.y))
                    newHover = TaskbarHitRegion::TrayVolume;
                else if (m_layout.trayNetwork.Contains(pos.x, pos.y))
                    newHover = TaskbarHitRegion::TrayNetwork;
                else if (m_layout.trayBattery.Contains(pos.x, pos.y))
                    newHover = TaskbarHitRegion::TrayBattery;
                else if (m_layout.trayNotifications.Contains(pos.x, pos.y))
                    newHover = TaskbarHitRegion::TrayNotifications;
                else if (m_layout.clockArea.Contains(pos.x, pos.y))
                    newHover = TaskbarHitRegion::Clock;
            }
        }
    }

    m_hoveredRegion = newHover;

    // ── Handle clicks ────────────────────────────────────────────────────
    if (m_pMouse->WasLeftClicked())
    {
        // Clear pressed state on all task items
        for (auto& ti : m_taskItems) { ti.isPressed = false; }

        if (m_hoveredRegion == TaskbarHitRegion::StartButton)
        {
            m_pressedRegion = TaskbarHitRegion::StartButton;

            if (m_pStartMenu)
            {
                m_pStartMenu->Toggle();
            }
        }
        else if (m_hoveredRegion == TaskbarHitRegion::SearchButton)
        {
            m_pressedRegion = TaskbarHitRegion::SearchButton;

            if (m_toggleSearch)
            {
                m_toggleSearch();
            }
        }
        else if (m_hoveredRegion == TaskbarHitRegion::TrayNotifications)
        {
            m_pressedRegion = TaskbarHitRegion::TrayNotifications;

            if (m_toggleNotifications)
            {
                m_toggleNotifications();
            }
        }
        else if (m_hoveredRegion == TaskbarHitRegion::TaskItem)
        {
            // Find which task item was clicked
            for (auto& ti : m_taskItems)
            {
                if (ti.isHovered)
                {
                    ti.isPressed = true;
                    // Find and focus/restore the target window
                    for (const auto& w : m_pWindowManager->GetCollection().GetAll())
                    {
                        if (w->GetId() == ti.windowId)
                        {
                            if (w->GetState() == WindowManager::WindowState::Minimized)
                            {
                                w->Restore();
                            }
                            m_pWindowManager->SetFocusedWindow(w.get());
                            m_pWindowManager->BringToFront(w.get());
                            break;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            m_pressedRegion = TaskbarHitRegion::None;
        }
    }

    // Handle release
    if (m_pMouse->WasReleased(Input::MouseButton::Left))
    {
        m_pressedRegion = TaskbarHitRegion::None;
        for (auto& ti : m_taskItems) { ti.isPressed = false; }
    }
}

// ============================================================================
//  Rendering
// ============================================================================

void Taskbar::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized) { return; }

    RenderBackground(renderer);
    RenderStartButton(renderer);
    RenderSearchButton(renderer);
    RenderTaskItems(renderer);
    RenderSystemTray(renderer);
    RenderActivityIndicator(renderer);
    RenderClock(renderer);
}

void Taskbar::RenderBackground(Graphics::Renderer& renderer) noexcept
{
    const auto& bgColor = m_pThemeManager->GetColor(Theme::SemanticColor::TaskbarBackground);
    const Graphics::Color bg{ bgColor.r, bgColor.g, bgColor.b, bgColor.a };

    const auto& rect = m_layout.bar;
    const D2D1_RECT_F d2dRect = D2D1::RectF(rect.x, rect.y, rect.Right(), rect.Bottom());

    renderer.FillRectangle(d2dRect, bg);

    // Subtle top border line
    const auto& borderColor = m_pThemeManager->GetColor(Theme::SemanticColor::WindowBorder);
    const Graphics::Color bCol{ borderColor.r, borderColor.g, borderColor.b, 0.4f };
    renderer.DrawLine(
        D2D1::Point2F(rect.x, rect.y),
        D2D1::Point2F(rect.Right(), rect.y),
        bCol, 1.0f);
}

void Taskbar::RenderStartButton(Graphics::Renderer& renderer) noexcept
{
    const auto& btn = m_layout.startButton;
    const D2D1_RECT_F d2dBtn = D2D1::RectF(btn.x, btn.y, btn.Right(), btn.Bottom());

    // Choose color based on state
    Theme::SemanticColor colorToken;
    if (m_pressedRegion == TaskbarHitRegion::StartButton)
    {
        colorToken = Theme::SemanticColor::StartButtonPressed;
    }
    else if (m_hoveredRegion == TaskbarHitRegion::StartButton)
    {
        colorToken = Theme::SemanticColor::StartButtonHover;
    }
    else
    {
        colorToken = Theme::SemanticColor::StartButtonBackground;
    }

    const auto& tc = m_pThemeManager->GetColor(colorToken);
    const Graphics::Color btnColor{ tc.r, tc.g, tc.b, tc.a };

    // Rounded rectangle for start button
    renderer.FillRectangle(d2dBtn, btnColor);

    // Draw "D" letter as placeholder DragonOS logo
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color logoColor{ textCol.r, textCol.g, textCol.b, textCol.a };
    const D2D1_RECT_F textRect = D2D1::RectF(
        btn.x + 2.0f, btn.y + 2.0f,
        btn.Right() - 2.0f, btn.Bottom() - 2.0f);
    renderer.DrawText(L"D", textRect, logoColor);

    // Hover glow overlay
    if (m_startBtnHoverAnim > 0.001f)
    {
        const Graphics::Color glow{ 1.0f, 1.0f, 1.0f, 0.1f * m_startBtnHoverAnim };
        renderer.FillRectangle(d2dBtn, glow);
    }
}

void Taskbar::RenderSearchButton(Graphics::Renderer& renderer) noexcept
{
    const auto& btn = m_layout.searchButton;
    const D2D1_RECT_F d2dBtn = D2D1::RectF(btn.x, btn.y, btn.Right(), btn.Bottom());

    const bool hovered = (m_hoveredRegion == TaskbarHitRegion::SearchButton);
    const bool pressed = (m_pressedRegion == TaskbarHitRegion::SearchButton);

    if (hovered || pressed)
    {
        auto token = pressed ? Theme::SemanticColor::TaskbarItemHover
                             : Theme::SemanticColor::TaskbarItemHover;
        const auto& hc = m_pThemeManager->GetColor(token);
        const Graphics::Color hoverBg{ hc.r, hc.g, hc.b, hc.a };
        renderer.FillRectangle(d2dBtn, hoverBg);
    }

    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color iconCol{ textCol.r, textCol.g, textCol.b, textCol.a };
    const D2D1_RECT_F textRect = D2D1::RectF(
        btn.x, btn.y + 2.0f, btn.Right(), btn.Bottom() - 2.0f);
    renderer.DrawText(L"\u2315", textRect, iconCol);
}

void Taskbar::RenderActivityIndicator(Graphics::Renderer& renderer) noexcept
{
    if (!m_pSvcMgr || !m_pSvcMgr->HasActiveServices()) { return; }

    const auto& indColor = m_pThemeManager->GetColor(Theme::SemanticColor::ServiceIndicator);
    const Graphics::Color ic{ indColor.r, indColor.g, indColor.b, indColor.a };

    const auto& act = m_layout.activityIndicator;
    const D2D1_RECT_F dot = D2D1::RectF(act.x, act.y, act.Right(), act.Bottom());

    const float pulse = 0.5f + 0.5f * std::sinf(static_cast<float>(std::chrono::steady_clock::now().time_since_epoch().count()) * 0.005f);
    const Graphics::Color pulseCol{ ic.r, ic.g, ic.b, ic.a * (0.4f + 0.6f * pulse) };
    renderer.FillRectangle(dot, pulseCol);
}

void Taskbar::RenderTaskItems(Graphics::Renderer& renderer) noexcept
{
    for (const auto& ti : m_taskItems)
    {
        RenderTaskItem(renderer, ti);
    }
}

void Taskbar::RenderTaskItem(
    Graphics::Renderer& renderer,
    const TaskItem&     item) noexcept
{
    const auto& rect = item.bounds;
    const D2D1_RECT_F d2dRect = D2D1::RectF(rect.x, rect.y, rect.Right(), rect.Bottom());

    // ── Active indicator (bottom accent line) ────────────────────────────
    if (item.isActive)
    {
        const auto& activeColor = m_pThemeManager->GetColor(Theme::SemanticColor::TaskbarItemActive);
        const Graphics::Color ac{ activeColor.r, activeColor.g, activeColor.b, activeColor.a };

        const D2D1_RECT_F activeBar = D2D1::RectF(
            rect.x + 4.0f, rect.Bottom() - 3.0f,
            rect.Right() - 4.0f, rect.Bottom());
        renderer.FillRectangle(activeBar, ac);
    }

    // ── Hover background ─────────────────────────────────────────────────
    if (item.hoverAnim > 0.001f)
    {
        const auto& hoverColor = m_pThemeManager->GetColor(Theme::SemanticColor::TaskbarItemHover);
        const Graphics::Color hc{ hoverColor.r, hoverColor.g, hoverColor.b,
                                   hoverColor.a * item.hoverAnim };
        renderer.FillRectangle(d2dRect, hc);
    }

    // ── Text ─────────────────────────────────────────────────────────────
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a };

    const D2D1_RECT_F textRect = D2D1::RectF(
        rect.x + 4.0f, rect.y + 2.0f,
        rect.Right() - 4.0f, rect.Bottom() - 2.0f);
    renderer.DrawText(item.title, textRect, tc);
}

void Taskbar::RenderSystemTray(Graphics::Renderer& renderer) noexcept
{
    const auto& secColor = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color iconCol{ secColor.r, secColor.g, secColor.b, secColor.a };

    const auto drawTrayIcon = [&](const Input::Bounds& b, const wchar_t* label)
    {
        const D2D1_RECT_F r = D2D1::RectF(b.x, b.y, b.Right(), b.Bottom());
        renderer.DrawText(label, r, iconCol);
    };

    drawTrayIcon(m_layout.trayVolume,   L"Vol");
    drawTrayIcon(m_layout.trayNetwork,  L"Net");
    drawTrayIcon(m_layout.trayBattery,  L"Bat");
    drawTrayIcon(m_layout.trayNotifications, L"Ntf");

    if (m_pNotifMgr && m_pNotifMgr->HasUnread())
    {
        const auto& accCol = m_pThemeManager->GetColor(Theme::SemanticColor::Accent);
        const Graphics::Color badgeCol{ accCol.r, accCol.g, accCol.b, accCol.a };

        const auto& nt = m_layout.trayNotifications;
        const float badgeSize = 8.0f;
        const D2D1_RECT_F badge = D2D1::RectF(
            nt.Right() - badgeSize, nt.y - 2.0f,
            nt.Right() + 2.0f, nt.y + badgeSize);
        renderer.FillRectangle(badge, badgeCol);
    }
}

void Taskbar::RenderClock(Graphics::Renderer& renderer) noexcept
{
    const auto& textCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextPrimary);
    const Graphics::Color tc{ textCol.r, textCol.g, textCol.b, textCol.a };

    // Time
    const D2D1_RECT_F timeRect = D2D1::RectF(
        m_layout.clockArea.x + 4.0f,
        m_layout.clockArea.y + 4.0f,
        m_layout.clockArea.Right() - 4.0f,
        m_layout.clockArea.y + m_layout.clockArea.height * 0.5f);
    renderer.DrawText(m_timeString, timeRect, tc);

    // Date (smaller, secondary)
    const auto& secCol = m_pThemeManager->GetColor(Theme::SemanticColor::TextSecondary);
    const Graphics::Color sc{ secCol.r, secCol.g, secCol.b, secCol.a };
    const D2D1_RECT_F dateRect = D2D1::RectF(
        m_layout.clockArea.x + 4.0f,
        m_layout.clockArea.y + m_layout.clockArea.height * 0.5f,
        m_layout.clockArea.Right() - 4.0f,
        m_layout.clockArea.Bottom() - 4.0f);
    renderer.DrawText(m_dateString, dateRect, sc);
}

} // namespace DragonOS::Taskbar
