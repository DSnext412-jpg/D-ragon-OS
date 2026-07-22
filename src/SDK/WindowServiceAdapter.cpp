#include "WindowServiceAdapter.hpp"

#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::SDK {

void PluginWindow::Show() noexcept
{
    m_internalWindow.Open();
}

void PluginWindow::Hide() noexcept
{
    m_internalWindow.Close();
}

void PluginWindow::Close() noexcept
{
    if (m_onClose)
    {
        m_onClose(m_internalWindow.GetId());
    }
    m_internalWindow.Close();
}

dragonos::sdk::IWindow* WindowServiceAdapter::Create(
    const dragonos::sdk::WindowCreateParams& params) noexcept
{
    auto internalWnd = std::make_unique<WindowManager::DragonWindow>(
        params.title,
        100.0f, 100.0f,
        params.width,
        params.height);

    auto* rawPtr = internalWnd.get();

    auto* added = m_windowManager.AddWindow(std::move(internalWnd));
    if (!added) { return nullptr; }

    added->Open();

    auto pluginWnd = std::make_unique<PluginWindow>(
        m_windowManager, *added);

    auto id = pluginWnd->GetId();
    auto* result = pluginWnd.get();
    m_windows[id] = std::move(pluginWnd);
    return result;
}

bool WindowServiceAdapter::DestroyWindow(uint64_t id) noexcept
{
    auto it = m_windows.find(id);
    if (it == m_windows.end()) { return false; }

    auto* internal = &it->second->GetInternal();
    m_windows.erase(it);
    m_windowManager.RemoveWindow(internal);
    return true;
}

dragonos::sdk::IWindow* WindowServiceAdapter::FindWindow(uint64_t id) noexcept
{
    auto it = m_windows.find(id);
    return (it != m_windows.end()) ? it->second.get() : nullptr;
}

dragonos::sdk::IWindow* WindowServiceAdapter::GetFocusedWindow() noexcept
{
    for (auto& [id, wnd] : m_windows)
    {
        if (wnd->GetInternal().IsFocused())
        {
            return wnd.get();
        }
    }
    return nullptr;
}

void WindowServiceAdapter::Shutdown() noexcept
{
    while (!m_windows.empty())
    {
        DestroyWindow(m_windows.begin()->first);
    }
}

} // namespace DragonOS::SDK
