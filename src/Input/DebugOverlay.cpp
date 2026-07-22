#include <Input/DebugOverlay.hpp>
#include <Input/InputManager.hpp>
#include <Input/MouseManager.hpp>
#include <Input/CursorManager.hpp>
#include <Input/HitTest.hpp>
#include <WindowManager/WindowManager.hpp>
#include <AppRuntime/ApplicationManager.hpp>
#include <Process/ProcessManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>

#include <d2d1.h>
#include <cwchar>

namespace DragonOS::Input {

bool DebugOverlay::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    return true;
}

void DebugOverlay::Shutdown() noexcept
{
}

void DebugOverlay::Update(float deltaTime) noexcept
{
    m_frameTime = deltaTime;

    m_fpsAccum += deltaTime;
    ++m_frameCount;

    if (m_fpsAccum >= 1.0f)
    {
        m_fps       = m_frameCount;
        m_frameCount = 0;
        m_fpsAccum   = 0.0f;
    }
}

void DebugOverlay::Render(Engine::EngineContext& ctx) noexcept
{
#ifdef _DEBUG
    auto* renderer = ctx.GetRenderer();
    if (!renderer) { return; }

    auto* target = renderer->GetRenderTarget();
    if (!target) { return; }

    // ── Gather data ────────────────────────────────────────────────────────
    float       mouseX = 0.0f, mouseY = 0.0f;
    float       wheel  = 0.0f;
    bool        inClient = false, dragging = false;

    CursorType  cursorType = CursorType::Arrow;

    if (m_pInput)
    {
        const auto& mm = m_pInput->GetMouseManager();
        mouseX   = mm.GetX();
        mouseY   = mm.GetY();
        wheel    = mm.GetWheelDelta();
        inClient = mm.IsInClient();
        dragging = mm.IsDragging();
        cursorType = m_pInput->GetCursorManager().GetCurrentCursor();
    }

    const wchar_t* hoveredName = L"(none)";
    if (m_pWindowManager)
    {
        auto* hov = m_pWindowManager->GetHoveredWindow();
        if (hov) { hoveredName = hov->GetTitle().c_str(); }
    }

    // ── Gather running applications and processes ─────────────────────────
    size_t appCount = 0;
    size_t procCount = 0;
    std::wstring appList;
    std::wstring procList;

    if (m_pAppMgr)
    {
        const auto apps = m_pAppMgr->GetAll();
        appCount = apps.size();
        for (const auto* app : apps)
        {
            if (!appList.empty()) { appList += L", "; }
            appList += app->GetDisplayName();
        }
    }

    if (m_pProcMgr)
    {
        const auto procs = m_pProcMgr->GetAll();
        procCount = procs.size();
        for (const auto* proc : procs)
        {
            if (!procList.empty()) { procList += L", "; }
            procList += proc->GetName();
        }
    }

    // ── Memory usage ───────────────────────────────────────────────────────
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(mem);
    ::GlobalMemoryStatusEx(&mem);

    // ── Build overlay text ─────────────────────────────────────────────────
    wchar_t buf[1024];
    const int len = ::swprintf_s(buf,
        L"FPS: %u  Frame: %.2f ms\n"
        L"Mouse: %.0f, %.0f  Wheel: %.0f  %s%s\n"
        L"Hover: %s  Cursor: %d\n"
        L"Apps: %zu  Procs: %zu\n"
        L"Mem: %llu MB / %llu MB\n"
        L"Running: %s\n"
        L"Processes: %s",
        m_fps, m_frameTime * 1000.0f,
        mouseX, mouseY, wheel,
        inClient ? L"IN" : L"OUT",
        (m_pInput && m_pInput->IsMousePressed(Input::MouseButton::Left)) ? L" [L]" : L"",
        hoveredName,
        static_cast<int>(cursorType),
        appCount, procCount,
        (mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024),
        mem.ullTotalPhys / (1024 * 1024),
        appList.c_str(),
        procList.c_str());

    if (len <= 0) { return; }

    const float overlayWidth = 420.0f;
    const float overlayHeight = 180.0f;
    const D2D1_RECT_F bgRect = D2D1::RectF(4.0f, 4.0f, 4.0f + overlayWidth, 4.0f + overlayHeight);

    auto* bgBrush = renderer->GetBrush({ 0.0f, 0.0f, 0.0f, 0.55f });
    if (bgBrush)
    {
        target->FillRectangle(&bgRect, bgBrush);
    }

    const D2D1_RECT_F textRect = D2D1::RectF(10.0f, 8.0f, 410.0f, 175.0f);
    renderer->DrawText(buf, textRect, { 0.0f, 1.0f, 0.0f, 1.0f });
#else
    (void)ctx;
#endif
}

void DebugOverlay::Resize(float /*width*/, float /*height*/) noexcept
{
}

}
