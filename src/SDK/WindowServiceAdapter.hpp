#pragma once

#include <DragonOS/Window.hpp>

#include <WindowManager/WindowManager.hpp>

#include <map>
#include <memory>

#ifdef CreateWindow
#undef CreateWindow
#endif
namespace DragonOS::SDK {

class PluginWindow final : public dragonos::sdk::IWindow {
public:
    PluginWindow(
        WindowManager::WindowManager& wm,
        WindowManager::DragonWindow&  internalWnd) noexcept
        : m_windowManager{ wm }
        , m_internalWindow{ internalWnd }
    {
    }

    uint64_t GetId() const noexcept override { return m_internalWindow.GetId(); }
    void SetTitle(std::wstring_view title) noexcept override { m_internalWindow.SetTitle(title); }
    std::wstring GetTitle() const noexcept override { return m_internalWindow.GetTitle(); }

    void Move(float x, float y) noexcept override { m_internalWindow.Move(x, y); }
    void Resize(float width, float height) noexcept override { m_internalWindow.Resize(width, height); }
    dragonos::sdk::WindowBounds GetBounds() const noexcept override
    {
        return { m_internalWindow.GetX(), m_internalWindow.GetY(),
                 m_internalWindow.GetWidth(), m_internalWindow.GetHeight() };
    }

    void Show() noexcept override;
    void Hide() noexcept override;
    void Close() noexcept override;
    void Focus() noexcept override { m_internalWindow.Focus(); }

    void Minimize() noexcept override { m_internalWindow.Minimize(); }
    void Maximize() noexcept override { m_internalWindow.Maximize(); }
    void Restore() noexcept override { m_internalWindow.Restore(); }
    dragonos::sdk::WindowState GetState() const noexcept override { return static_cast<dragonos::sdk::WindowState>(m_internalWindow.GetState()); }

    void SetMinWidth(float w) noexcept override { m_internalWindow.SetMinWidth(w); }
    void SetMinHeight(float h) noexcept override { m_internalWindow.SetMinHeight(h); }

    void SetOnClose(CloseCallback cb) noexcept override
    {
        m_onClose = std::move(cb);
    }

    WindowManager::DragonWindow& GetInternal() noexcept { return m_internalWindow; }

private:
    WindowManager::WindowManager& m_windowManager;
    WindowManager::DragonWindow&  m_internalWindow;
    dragonos::sdk::IWindow::CloseCallback m_onClose;
};

class WindowServiceAdapter final : public dragonos::sdk::IWindowService {
public:
    explicit WindowServiceAdapter(
        WindowManager::WindowManager& wm) noexcept
        : m_windowManager{ wm }
    {
    }

    dragonos::sdk::IWindow* Create(
        const dragonos::sdk::WindowCreateParams& params) noexcept override;
    bool DestroyWindow(uint64_t id) noexcept override;
    dragonos::sdk::IWindow* FindWindow(uint64_t id) noexcept override;
    dragonos::sdk::IWindow* GetFocusedWindow() noexcept override;

    ~WindowServiceAdapter() noexcept { Shutdown(); }

private:
    void Shutdown() noexcept;

    WindowManager::WindowManager& m_windowManager;
    std::map<uint64_t, std::unique_ptr<PluginWindow>> m_windows;
};

} // namespace DragonOS::SDK
