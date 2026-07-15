/**
 * @file    Window.cpp
 * @brief   Implementation of the Window class.
 */

#include <Engine/Engine.hpp>
#include <Graphics/Color.hpp>
#include <Input/InputManager.hpp>
#include <Window/Window.hpp>

#include <exception>
#include <stdexcept>

namespace DragonOS::Window {

// ============================================================================
//  Construction / Destruction
// ============================================================================

Window::Window(
    HINSTANCE        hInstance,
    std::wstring     className,
    std::wstring_view title,
    int              width,
    int              height)
    : m_hInstance{ hInstance }
    , m_className{ std::move(className) }
{
    // ── Register window class ──────────────────────────────────────────────
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = m_hInstance;
    wc.hIcon         = ::LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor       = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = m_className.c_str();

    if (!::RegisterClassExW(&wc))
    {
        throw std::runtime_error{ "Window: RegisterClassExW failed." };
    }

    // ── Create native window ───────────────────────────────────────────────
    // Pass 'this' as lpParam so WndProc can retrieve the instance during
    // WM_NCCREATE and store it in the window's GWLP_USERDATA.
    m_hWnd = ::CreateWindowExW(
        0,                      // dwExStyle
        m_className.c_str(),    // lpClassName
        title.data(),           // lpWindowName
        WS_OVERLAPPEDWINDOW,    // dwStyle
        CW_USEDEFAULT,          // X
        CW_USEDEFAULT,          // Y
        width,                  // nWidth
        height,                 // nHeight
        nullptr,                // hWndParent
        nullptr,                // hMenu
        m_hInstance,            // hInstance
        this);                  // lpCreateParams

    if (!m_hWnd)
    {
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
        throw std::runtime_error{ "Window: CreateWindowExW failed." };
    }

    // ── Initialise the Direct2D renderer ───────────────────────────────────
    if (!m_renderer.Initialize(m_hWnd))
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
        throw std::runtime_error{ "Window: Failed to initialise renderer." };
    }

    // ── Initialise the desktop manager ─────────────────────────────────────
    if (!m_desktopManager.Initialize())
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
        throw std::runtime_error{ "Window: Failed to initialise desktop." };
    }

    // ── Store client dimensions ────────────────────────────────────────────
    m_width  = width;
    m_height = height;
}

Window::~Window() noexcept
{
    Close();

    if (!m_className.empty())
    {
        ::UnregisterClassW(m_className.c_str(), m_hInstance);
    }
}

// ============================================================================
//  Public interface
// ============================================================================

void Window::Show(int nCmdShow) noexcept
{
    if (m_hWnd)
    {
        ::ShowWindow(m_hWnd, nCmdShow);
    }
}

void Window::Update() noexcept
{
    if (m_hWnd)
    {
        ::UpdateWindow(m_hWnd);
    }
}

void Window::Close() noexcept
{
    if (m_hWnd && ::IsWindow(m_hWnd))
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

HWND Window::Handle() const noexcept
{
    return m_hWnd;
}

// ============================================================================
//  Static window procedure
// ============================================================================

LRESULT CALLBACK Window::WndProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    // On the very first message (WM_NCCREATE), attach the Window instance
    // to the window's user-data slot so subsequent calls can find it.
    if (uMsg == WM_NCCREATE)
    {
        auto* pcs  = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* pWin = static_cast<Window*>(pcs->lpCreateParams);

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pWin));
    }

    // Retrieve the instance pointer and forward the message.
    auto* pWindow = reinterpret_cast<Window*>(
        ::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (pWindow)
    {
        return pWindow->HandleMessage(uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// ============================================================================
//  Instance message dispatcher
// ============================================================================

LRESULT Window::HandleMessage(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    // ── Core window messages ─────────────────────────────────────────────
    case WM_CLOSE:   OnClose();   return 0;
    case WM_DESTROY: OnDestroy(); return 0;
    case WM_PAINT:   OnPaint();   return 0;
    case WM_SIZE:    OnResize(lParam); return 0;

    // ── Input messages → forwarded to InputManager ──────────────────────
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_KEYDOWN:  case WM_KEYUP:
    case WM_SYSKEYDOWN: case WM_SYSKEYUP:
    case WM_CHAR:
        if (m_pInputManager)
        {
            m_pInputManager->HandleWin32Message(uMsg, wParam, lParam);
        }
        return 0;

    default:
        return ::DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
    }
}

// ============================================================================
//  Message handlers
// ============================================================================

void Window::OnClose() noexcept
{
    ::DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
}

void Window::OnDestroy() noexcept
{
    ::PostQuitMessage(0);
}

void Window::OnPaint() noexcept
{
    PAINTSTRUCT ps{};
    ::BeginPaint(m_hWnd, &ps);

    if (m_pEngine)
    {
        m_pEngine->Render();
    }

    ::EndPaint(m_hWnd, &ps);
}

void Window::OnResize(LPARAM lParam) noexcept
{
    const UINT cx = LOWORD(lParam);
    const UINT cy = HIWORD(lParam);

    if (cx > 0 && cy > 0)
    {
        m_renderer.Resize(cx, cy);

        if (m_pEngine)
        {
            m_pEngine->Resize(
                static_cast<float>(cx),
                static_cast<float>(cy));
        }

        m_width  = cx;
        m_height = cy;
    }
}

} // namespace DragonOS::Window
