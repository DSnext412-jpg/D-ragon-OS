#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/SnapLayout.hpp>
#include <WindowManager/WindowAnimations.hpp>
#include <Animation/AnimationManager.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseButtons.hpp>
#include <Input/MouseManager.hpp>

#include <algorithm>

namespace DragonOS::WindowManager {

// ============================================================================
//  Lifecycle
// ============================================================================

WindowManager::~WindowManager() noexcept = default;

bool WindowManager::Initialize() noexcept
{
    if (m_initialized) { return true; }

    auto explorer = std::make_unique<DragonWindow>(
        L"Explorer",
        120.0f, 100.0f, 800.0f, 500.0f);

    explorer->Focus();
    m_pFocused = AddWindow(std::move(explorer));

    m_initialized = true;
    return true;
}

void WindowManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_isDragging   = false;
    m_isResizing   = false;
    m_pDragWindow  = nullptr;
    m_pResizeWindow = nullptr;
    m_pFocused     = nullptr;
    m_pHovered     = nullptr;
    m_collection.Clear();
    m_initialized  = false;
}

// ============================================================================
//  Per-frame
// ============================================================================

void WindowManager::Render(
    Graphics::Renderer& renderer,
    float               viewportWidth,
    float               viewportHeight) noexcept
{
    if (!m_initialized) { return; }

    m_viewportWidth  = viewportWidth;
    m_viewportHeight = viewportHeight;

    m_collection.Render(renderer);

    RenderSnapIndicator(renderer);
}

void WindowManager::RenderSnapIndicator(Graphics::Renderer& renderer) noexcept
{
    if (m_activeSnapRegion == SnapRegion::None || !m_isDragging)
    {
        return;
    }

    const auto bounds = SnapLayout::GetSnapBounds(
        m_activeSnapRegion, m_viewportWidth, m_viewportHeight);

    const D2D1_RECT_F rect = D2D1::RectF(
        bounds.x, bounds.y,
        bounds.x + bounds.width, bounds.y + bounds.height);

    const Graphics::Color indicatorColor{ 58.0f / 255.0f, 134.0f / 255.0f, 255.0f / 255.0f, 0.20f };
    const Graphics::Color borderColor{ 58.0f / 255.0f, 134.0f / 255.0f, 255.0f / 255.0f, 0.50f };

    renderer.FillRectangle(rect, indicatorColor);
    renderer.DrawRectangle(rect, borderColor, 2.0f);
}

void WindowManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    ProcessInput();

    if (m_isDragging && m_pDragWindow)
    {
        const auto pos = m_pMouse->GetPosition();

        UpdateDrag(pos.x, pos.y);

        m_activeSnapRegion = SnapLayout::DetectSnapRegion(
            m_viewportWidth, m_viewportHeight,
            m_pDragWindow->GetX(), m_pDragWindow->GetY(),
            m_pDragWindow->GetWidth(), m_pDragWindow->GetHeight());
    }

    m_collection.Update(deltaTime);
}

void WindowManager::Resize(float width, float height) noexcept
{
    m_viewportWidth  = width;
    m_viewportHeight = height;

    for (auto& w : m_collection.GetAll())
    {
        if (w->GetState() == WindowState::Maximized)
        {
            w->Move(0.0f, 0.0f);
            w->Resize(width, height);
        }
    }
}

// ============================================================================
//  Input processing
// ============================================================================

void WindowManager::ProcessInput() noexcept
{
    if (!m_pMouse) { return; }

    m_events.clear();

    for (auto& w : m_collection.GetAll())
    {
        w->SetEnteredThisFrame(false);
        w->SetExitedThisFrame(false);
    }

    const auto pos = m_pMouse->GetPosition();

    // ── Hit test ──────────────────────────────────────────────────────────
    DragonWindow* hitWnd = nullptr;
    Input::HitTestRegion region = Input::HitTestRegion::None;

    if (m_isDragging && m_pDragWindow)
    {
        hitWnd = m_pDragWindow;
        region = Input::HitTestRegion::TitleBar;
    }
    else if (m_isResizing && m_pResizeWindow)
    {
        hitWnd = m_pResizeWindow;
        region = m_resizeEdge;
    }
    else if (m_pMouse->IsInClient())
    {
        hitWnd = m_collection.HitTest(pos.x, pos.y);
        if (hitWnd)
        {
            region = hitWnd->HitTest(pos.x, pos.y);
        }
    }

    m_hitRegion = region;

    UpdateControlHover(hitWnd, region);

    // ── Hover state ───────────────────────────────────────────────────────
    if (hitWnd != m_pHovered)
    {
        if (m_pHovered)
        {
            m_pHovered->SetHovered(false);
            m_pHovered->SetExitedThisFrame(true);
            PushEvent({ Input::UIEventType::MouseExit, pos.x, pos.y });
        }

        if (hitWnd)
        {
            hitWnd->SetHovered(true);
            hitWnd->SetEnteredThisFrame(true);
            PushEvent({ Input::UIEventType::MouseEnter, pos.x, pos.y,
                        Input::MouseButton::Left, region, hitWnd->GetId() });
        }

        m_pHovered = hitWnd;
    }

    if (m_pHovered)
    {
        PushEvent({ Input::UIEventType::MouseMove, pos.x, pos.y,
                    Input::MouseButton::Left, region, m_pHovered->GetId() });
    }

    // ── Drag / Resize update (during hold) ────────────────────────────────
    if (m_pMouse->IsHeld(Input::MouseButton::Left))
    {
        if (m_isDragging && m_pDragWindow)  { UpdateDrag(pos.x, pos.y); }
        if (m_isResizing && m_pResizeWindow) { UpdateResize(pos.x, pos.y); }
    }

    // ── Button events for ALL buttons (including Left) ────────────────────
    if (hitWnd)
    {
        for (std::size_t i = 0; i < Input::MouseButtonCount; ++i)
        {
            const auto btn = static_cast<Input::MouseButton>(i);

            if (m_pMouse->WasClicked(btn))
            {
                hitWnd->SetPressed(true);
                PushEvent({ Input::UIEventType::MouseDown, pos.x, pos.y,
                            btn, region, hitWnd->GetId() });

                if (btn == Input::MouseButton::Left && hitWnd != m_pFocused)
                {
                    SetFocusedWindow(hitWnd);
                }
            }

            if (m_pMouse->WasReleased(btn))
            {
                hitWnd->SetPressed(false);
                PushEvent({ Input::UIEventType::MouseUp, pos.x, pos.y,
                            btn, region, hitWnd->GetId() });
                PushEvent({ Input::UIEventType::MouseClick, pos.x, pos.y,
                            btn, region, hitWnd->GetId() });
            }

            if (m_pMouse->WasDoubleClicked(btn))
            {
                PushEvent({ Input::UIEventType::MouseDoubleClick, pos.x, pos.y,
                            btn, region, hitWnd->GetId() });
            }
        }
    }
    else
    {
        if (!m_pMouse->IsHeld(Input::MouseButton::Left) &&
            !m_pMouse->IsHeld(Input::MouseButton::Right))
        {
            for (auto& w : m_collection.GetAll())
            {
                w->SetPressed(false);
            }
        }
    }

    // ── Drag / Resize / Control start (left button) ──────────────────────
    if (m_pMouse->WasClicked(Input::MouseButton::Left) && hitWnd)
    {
        if (region == Input::HitTestRegion::TitleBar &&
            HasFlag(hitWnd->GetStyle(), WindowStyle::Movable))
        {
            StartDrag(hitWnd, pos.x, pos.y);
        }
        else if (IsResizeRegion(region) &&
                 HasFlag(hitWnd->GetStyle(), WindowStyle::Resizable))
        {
            StartResize(hitWnd, region, pos.x, pos.y);
        }
        else if (region == Input::HitTestRegion::CloseButton &&
                 HasFlag(hitWnd->GetStyle(), WindowStyle::Closable))
        {
            HandleControlClick(hitWnd, region);
        }
        else if (region == Input::HitTestRegion::MaximizeButton &&
                 HasFlag(hitWnd->GetStyle(), WindowStyle::Resizable))
        {
            HandleControlClick(hitWnd, region);
        }
        else if (region == Input::HitTestRegion::MinimizeButton &&
                 HasFlag(hitWnd->GetStyle(), WindowStyle::Closable))
        {
            HandleControlClick(hitWnd, region);
        }
    }

    // ── Drag / Resize end ─────────────────────────────────────────────────
    if (m_pMouse->WasReleased(Input::MouseButton::Left))
    {
        if (m_isDragging) { EndDrag(); }
        if (m_isResizing) { EndResize(); }
    }

    // ── Double-click title bar to maximise ────────────────────────────────
    if (m_pMouse->WasDoubleClicked(Input::MouseButton::Left) && hitWnd)
    {
        if (region == Input::HitTestRegion::TitleBar &&
            HasFlag(hitWnd->GetStyle(), WindowStyle::Resizable))
        {
            ToggleMaximize(hitWnd);
        }
    }
}

// ============================================================================
//  Drag helpers
// ============================================================================

void WindowManager::StartDrag(
    DragonWindow* window, float mouseX, float mouseY) noexcept
{
    m_isDragging   = true;
    m_pDragWindow  = window;
    m_dragOffsetX  = mouseX - window->GetX();
    m_dragOffsetY  = mouseY - window->GetY();
    m_activeSnapRegion = SnapRegion::None;
}

void WindowManager::UpdateDrag(float mouseX, float mouseY) noexcept
{
    if (!m_pDragWindow) { return; }

    float newX = mouseX - m_dragOffsetX;
    float newY = mouseY - m_dragOffsetY;

    newX = (std::max)(0.0f, newX);
    newY = (std::max)(0.0f, newY);
    newX = (std::min)(newX, m_viewportWidth - m_pDragWindow->GetWidth());
    newY = (std::min)(newY, m_viewportHeight - m_pDragWindow->GetHeight());

    m_pDragWindow->Move(newX, newY);
}

void WindowManager::EndDrag() noexcept
{
    if (!m_pDragWindow) { return; }

    if (SnapLayout::IsValidSnap(m_activeSnapRegion))
    {
        const auto snapBounds = SnapLayout::GetSnapBounds(
            m_activeSnapRegion, m_viewportWidth, m_viewportHeight);

        m_pDragWindow->SetRestoreGeometry(
            m_pDragWindow->GetX(),
            m_pDragWindow->GetY(),
            m_pDragWindow->GetWidth(),
            m_pDragWindow->GetHeight());

        if (m_activeSnapRegion == SnapRegion::Full)
        {
            m_pDragWindow->Move(snapBounds.x, snapBounds.y);
            m_pDragWindow->Resize(snapBounds.width, snapBounds.height);
            m_pDragWindow->Maximize();
        }
        else
        {
            if (m_pAnimationManager)
            {
                auto anim = CreateTransformAnimation(
                    m_pDragWindow,
                    snapBounds.x, snapBounds.y,
                    snapBounds.width, snapBounds.height);
                m_pAnimationManager->Play(std::move(anim));
            }
            else
            {
                m_pDragWindow->Move(snapBounds.x, snapBounds.y);
                m_pDragWindow->Resize(snapBounds.width, snapBounds.height);
            }
        }
    }

    m_isDragging  = false;
    m_pDragWindow = nullptr;
    m_activeSnapRegion = SnapRegion::None;
}

// ============================================================================
//  Resize helpers
// ============================================================================

void WindowManager::StartResize(
    DragonWindow* window, Input::HitTestRegion edge,
    float mouseX, float mouseY) noexcept
{
    m_isResizing         = true;
    m_pResizeWindow      = window;
    m_resizeEdge         = edge;
    m_resizeAnchor       = window->GetBounds();
    m_resizeAnchorPoint  = { mouseX, mouseY };
}

void WindowManager::UpdateResize(float mouseX, float mouseY) noexcept
{
    if (!m_pResizeWindow) { return; }

    float dx = mouseX - m_resizeAnchorPoint.x;
    float dy = mouseY - m_resizeAnchorPoint.y;

    float newX = m_resizeAnchor.x;
    float newY = m_resizeAnchor.y;
    float newW = m_resizeAnchor.width;
    float newH = m_resizeAnchor.height;

    switch (m_resizeEdge)
    {
    case Input::HitTestRegion::BorderLeft:
        newX += dx;
        newW -= dx;
        break;
    case Input::HitTestRegion::BorderRight:
        newW += dx;
        break;
    case Input::HitTestRegion::BorderTop:
        newY += dy;
        newH -= dy;
        break;
    case Input::HitTestRegion::BorderBottom:
        newH += dy;
        break;
    case Input::HitTestRegion::BorderTopLeft:
        newX += dx;
        newY += dy;
        newW -= dx;
        newH -= dy;
        break;
    case Input::HitTestRegion::BorderTopRight:
        newY += dy;
        newW += dx;
        newH -= dy;
        break;
    case Input::HitTestRegion::BorderBottomLeft:
        newX += dx;
        newW -= dx;
        newH += dy;
        break;
    case Input::HitTestRegion::BorderBottomRight:
        newW += dx;
        newH += dy;
        break;
    default:
        break;
    }

    if (newW < DragonWindow::MinWindowWidth)
    {
        if (m_resizeEdge == Input::HitTestRegion::BorderLeft ||
            m_resizeEdge == Input::HitTestRegion::BorderTopLeft ||
            m_resizeEdge == Input::HitTestRegion::BorderBottomLeft)
        {
            newX = m_resizeAnchor.x + m_resizeAnchor.width - DragonWindow::MinWindowWidth;
        }
        newW = DragonWindow::MinWindowWidth;
    }

    if (newH < DragonWindow::MinWindowHeight)
    {
        if (m_resizeEdge == Input::HitTestRegion::BorderTop ||
            m_resizeEdge == Input::HitTestRegion::BorderTopLeft ||
            m_resizeEdge == Input::HitTestRegion::BorderTopRight)
        {
            newY = m_resizeAnchor.y + m_resizeAnchor.height - DragonWindow::MinWindowHeight;
        }
        newH = DragonWindow::MinWindowHeight;
    }

    m_pResizeWindow->Move(newX, newY);
    m_pResizeWindow->Resize(newW, newH);
}

void WindowManager::EndResize() noexcept
{
    m_isResizing    = false;
    m_pResizeWindow = nullptr;
    m_resizeEdge    = Input::HitTestRegion::None;
}

// ============================================================================
//  Control button helpers
// ============================================================================

void WindowManager::UpdateControlHover(
    DragonWindow* window, Input::HitTestRegion region) noexcept
{
    for (auto& w : m_collection.GetAll())
    {
        if (w.get() == window)
        {
            w->SetActiveControlRegion(region);
        }
        else
        {
            w->SetActiveControlRegion(Input::HitTestRegion::None);
            w->SetPressedControlRegion(Input::HitTestRegion::None);
        }
    }
}

void WindowManager::HandleControlClick(
    DragonWindow* window, Input::HitTestRegion region) noexcept
{
    if (!window) { return; }

    switch (region)
    {
    case Input::HitTestRegion::CloseButton:
        window->Close();
        break;

    case Input::HitTestRegion::MaximizeButton:
        ToggleMaximize(window);
        break;

    case Input::HitTestRegion::MinimizeButton:
        window->Minimize();
        break;

    default:
        break;
    }
}

// ============================================================================
//  Window management
// ============================================================================

DragonWindow* WindowManager::AddWindow(
    std::unique_ptr<DragonWindow> window) noexcept
{
    return m_collection.Add(std::move(window));
}

bool WindowManager::RemoveWindow(DragonWindow* window) noexcept
{
    if (window == m_pDragWindow)  { m_pDragWindow  = nullptr; m_isDragging = false; }
    if (window == m_pResizeWindow) { m_pResizeWindow = nullptr; m_isResizing = false; }

    const bool removed = m_collection.Remove(window);

    if (removed && m_pFocused == window) { m_pFocused = nullptr; }
    if (removed && m_pHovered == window) { m_pHovered = nullptr; }

    return removed;
}

void WindowManager::BringToFront(DragonWindow* window) noexcept
{
    m_collection.BringToFront(window);
}

void WindowManager::SendToBack(DragonWindow* window) noexcept
{
    m_collection.SendToBack(window);
}

DragonWindow* WindowManager::FindWindowByTitle(
    std::wstring_view title) noexcept
{
    return m_collection.Find(title);
}

void WindowManager::SetFocusedWindow(DragonWindow* window) noexcept
{
    if (m_pFocused && m_pFocused != window)
    {
        m_pFocused->FocusLost();
        m_pFocused->SetActiveControlRegion(Input::HitTestRegion::None);
        m_pFocused->SetPressedControlRegion(Input::HitTestRegion::None);
        PushEvent({ Input::UIEventType::FocusLost, 0, 0,
                    Input::MouseButton::Left, Input::HitTestRegion::None,
                    m_pFocused->GetId() });
    }

    m_pFocused = window;

    if (m_pFocused)
    {
        m_pFocused->Focus();
        BringToFront(m_pFocused);
        PushEvent({ Input::UIEventType::FocusGained, 0, 0,
                    Input::MouseButton::Left, Input::HitTestRegion::None,
                    m_pFocused->GetId() });
    }
}

void WindowManager::ToggleMaximize(DragonWindow* window) noexcept
{
    if (!window) { return; }

    if (window->GetState() == WindowState::Maximized)
    {
        window->Restore();

        const float rx = window->GetRestoreX();
        const float ry = window->GetRestoreY();
        const float rw = window->GetRestoreW();
        const float rh = window->GetRestoreH();

        if (rw > 0.0f && rh > 0.0f)
        {
            if (m_pAnimationManager)
            {
                auto anim = CreateTransformAnimation(
                    window, rx, ry, rw, rh);
                m_pAnimationManager->Play(std::move(anim));
            }
            else
            {
                window->Move(rx, ry);
                window->Resize(rw, rh);
            }
        }
    }
    else
    {
        window->SetRestoreGeometry(
            window->GetX(), window->GetY(),
            window->GetWidth(), window->GetHeight());

        window->Maximize();

        if (m_pAnimationManager)
        {
            auto anim = CreateTransformAnimation(
                window, 0.0f, 0.0f,
                m_viewportWidth, m_viewportHeight);
            m_pAnimationManager->Play(std::move(anim));
        }
        else
        {
            window->Move(0.0f, 0.0f);
            window->Resize(m_viewportWidth, m_viewportHeight);
        }
    }
}

} // namespace DragonOS::WindowManager
