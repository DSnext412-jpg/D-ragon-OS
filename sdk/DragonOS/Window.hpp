#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef FindWindow
#undef FindWindow
#endif

namespace dragonos::sdk {

struct WindowCreateParams {
    std::wstring title{ L"Plugin Window" };
    float x{ 100.0f }, y{ 100.0f };
    float width{ 800.0f }, height{ 600.0f };
    bool resizable{ true };
    bool closable{ true };
};

struct WindowBounds {
    float x{}, y{}, width{}, height{};
};

enum class WindowState { Normal, Minimized, Maximized };

class IWindow {
public:
    virtual ~IWindow() noexcept = default;

    virtual uint64_t GetId() const noexcept = 0;
    virtual void SetTitle(std::wstring_view title) noexcept = 0;
    virtual std::wstring GetTitle() const noexcept = 0;

    virtual void Move(float x, float y) noexcept = 0;
    virtual void Resize(float width, float height) noexcept = 0;
    virtual WindowBounds GetBounds() const noexcept = 0;

    virtual void Show() noexcept = 0;
    virtual void Hide() noexcept = 0;
    virtual void Close() noexcept = 0;
    virtual void Focus() noexcept = 0;

    virtual void Minimize() noexcept = 0;
    virtual void Maximize() noexcept = 0;
    virtual void Restore() noexcept = 0;
    virtual WindowState GetState() const noexcept = 0;

    virtual void SetMinWidth(float w) noexcept = 0;
    virtual void SetMinHeight(float h) noexcept = 0;

    using CloseCallback = std::function<void(uint64_t windowId)>;
    virtual void SetOnClose(CloseCallback callback) noexcept = 0;
};

class IWindowService {
public:
    virtual ~IWindowService() noexcept = default;
    virtual IWindow* Create(const WindowCreateParams& params) noexcept = 0;
    virtual bool DestroyWindow(uint64_t windowId) noexcept = 0;
    virtual IWindow* FindWindow(uint64_t windowId) noexcept = 0;
    virtual IWindow* GetFocusedWindow() noexcept = 0;
};

} // namespace dragonos::sdk
