# DragonOS SDK API Reference

## Namespace: `dragonos::sdk`

All public API types live in the `dragonos::sdk` namespace.

---

## Version

**Header:** `<DragonOS/Version.hpp>`

```cpp
#define DRAGONOS_SDK_VERSION_MAJOR 1
#define DRAGONOS_SDK_VERSION_MINOR 0
#define DRAGONOS_SDK_VERSION_PATCH 0
#define DRAGONOS_SDK_VERSION       10000

struct SDKVersion {
    static bool IsCompatible(int pluginSdkVersion);
};
```

---

## Logging

**Header:** `<DragonOS/Logging.hpp>`

```cpp
class Logger {
public:
    static Logger& Get();
    void Debug(std::wstring_view msg);
    void Info(std::wstring_view msg);
    void Warning(std::wstring_view msg);
    void Error(std::wstring_view msg);
};
```

---

## Notification

**Header:** `<DragonOS/Notification.hpp>`

```cpp
enum class NotificationSeverity { Information, Warning, Error, Success };

struct NotificationData {
    std::wstring title;
    std::wstring message;
    std::wstring source;
    NotificationSeverity severity;
};

class INotificationService {
    virtual uint64_t Show(const NotificationData& notif) = 0;
    virtual bool Dismiss(uint64_t id) = 0;
    virtual bool DismissGroup(std::wstring_view groupKey) = 0;
    virtual void DismissAll() = 0;
};
```

---

## Config

**Header:** `<DragonOS/Config.hpp>`

```cpp
class ConfigSection {
    void Set(std::wstring_view key, std::wstring_view value);
    std::wstring_view Get(std::wstring_view key, std::wstring_view defaultValue = {});
    int GetInt(std::wstring_view key, int defaultValue = 0);
    bool Has(std::wstring_view key);
};

class IConfigService {
    virtual ConfigSection& GetSection(std::wstring_view name) = 0;
    virtual bool Save() = 0;
    virtual bool Load() = 0;
};
```

---

## File

**Header:** `<DragonOS/File.hpp>`

```cpp
struct FileEntry {
    std::wstring name, fullPath;
    uint64_t size;
    bool isDirectory;
};

class IFileService {
    virtual std::vector<FileEntry> List(std::wstring_view path) = 0;
    virtual bool Exists(std::wstring_view path) = 0;
    virtual bool CreateFolder(std::wstring_view path) = 0;
    virtual bool DeleteFile(std::wstring_view path) = 0;
    virtual bool DeleteDirectory(std::wstring_view path, bool recursive) = 0;
    virtual std::wstring Combine(std::wstring_view a, std::wstring_view b) = 0;
    virtual std::wstring GetParent(std::wstring_view path) = 0;
    virtual std::wstring GetFileName(std::wstring_view path) = 0;
};
```

---

## Theme

**Header:** `<DragonOS/Theme.hpp>`

```cpp
struct ThemeColor { float r, g, b, a; };

enum class ThemeToken {
    DesktopBackground, WindowBackground, WindowTitleBar,
    TextPrimary, TextSecondary,
    Accent, AccentHover, AccentPressed,
};

class IThemeService {
    virtual ThemeColor GetColor(ThemeToken token) = 0;
    virtual bool IsDarkMode() = 0;
};
```

---

## Input

**Header:** `<DragonOS/Input.hpp>`

```cpp
enum class KeyCode { /* Win32 VK_* values */ };
enum class MouseButton { Left, Right, Middle };

struct MouseState { float x, y; bool leftPressed, rightPressed, middlePressed; };
struct InputEventData { /* tagged union for mouse/key events */ };

class IInputService {
    virtual MouseState GetMouseState() = 0;
    virtual bool IsKeyDown(KeyCode key) = 0;
    virtual bool WasKeyPressed(KeyCode key) = 0;
    virtual bool WasKeyReleased(KeyCode key) = 0;
    virtual uint32_t GetEventCount() = 0;
    virtual InputEventData GetEvent(uint32_t index) = 0;
};
```

---

## Resource

**Header:** `<DragonOS/Resource.hpp>`

```cpp
class IResourceService {
    virtual std::vector<uint8_t> LoadBinary(std::wstring_view path) = 0;
    virtual std::wstring LoadText(std::wstring_view path) = 0;
    virtual std::wstring ResolvePath(std::wstring_view relativePath) = 0;
    virtual bool Exists(std::wstring_view path) = 0;
};
```

---

## Window

**Header:** `<DragonOS/Window.hpp>`

```cpp
struct WindowCreateParams { std::wstring title; float width, height; bool resizable; };
enum class WindowState { Normal, Minimized, Maximized, FullScreen };

class IWindow {
    virtual uint64_t GetId() = 0;
    virtual std::wstring GetTitle() = 0;
    virtual void SetTitle(std::wstring_view title) = 0;
    virtual float GetX(), GetY(), GetWidth(), GetHeight();
    virtual void GetBounds(float& x, float& y, float& w, float& h);
    virtual void Move(float x, float y) = 0;
    virtual void Resize(float w, float h) = 0;
    virtual void Show(), Hide(), Close();
    virtual void Focus(), Minimize(), Maximize(), Restore();
    virtual WindowState GetState();
    virtual void SetMinWidth(float), SetMinHeight(float);
    using CloseCallback = std::function<void(uint64_t)>;
    virtual void SetOnClose(CloseCallback);
};

class IWindowService {
    virtual IWindow* Create(const WindowCreateParams& params) = 0;
    virtual bool DestroyWindow(uint64_t id) = 0;
    virtual IWindow* FindWindow(uint64_t id) = 0;
    virtual IWindow* GetFocusedWindow() = 0;
};
```

---

## Menu

**Header:** `<DragonOS/Menu.hpp>`

```cpp
struct MenuItem {
    uint64_t id; std::wstring label, shortcut;
    bool enabled, checked, separator;
    std::function<void()> action;
    std::vector<MenuItem> submenu;
};

class IMenuService {
    virtual uint64_t ShowContextMenu(
        const std::vector<MenuItem>& items, float x, float y) = 0;
    virtual void CloseMenu(uint64_t menuId) = 0;
};
```

---

## Dialog

**Header:** `<DragonOS/Dialog.hpp>`

```cpp
enum class DialogResult { OK, Cancel, Yes, No, Retry };
enum class DialogButtons { OK, OKCancel, YesNo, YesNoCancel, RetryCancel };
struct DialogParams { std::wstring title, message; DialogButtons buttons; };

class IDialogService {
    virtual DialogResult ShowMessageBox(const DialogParams& params) = 0;
    virtual void ShowMessageBoxAsync(const DialogParams& params) = 0;
    virtual std::wstring OpenFileDialog(std::wstring_view filter) = 0;
    virtual std::wstring SaveFileDialog(
        std::wstring_view defaultName, std::wstring_view filter) = 0;
    virtual std::wstring OpenFolderDialog() = 0;
};
```

---

## Events

**Header:** `<DragonOS/Events.hpp>`

```cpp
using EventHandlerId = uint64_t;
using EventCallback = std::function<void(const Event&)>;

enum class EventType {
    AppLaunched, WindowOpened, WindowClosed, WindowFocused,
    ThemeChanged, ConfigChanged, PluginLoaded, PluginUnloaded, Custom
};

struct Event { EventType type; std::wstring sourceName; uint64_t sourceId; };

class IEventHandler { virtual void OnEvent(const Event&) = 0; };

class IEventBus {
    virtual EventHandlerId Subscribe(
        EventType, EventCallback, int priority = 0) = 0;
    virtual EventHandlerId Subscribe(
        EventType, IEventHandler*, int priority = 0) = 0;
    virtual bool Unsubscribe(EventHandlerId) = 0;
    virtual void Publish(const Event&) = 0;
    virtual void PublishAsync(const Event&) = 0;
};
```

---

## Application

**Header:** `<DragonOS/Application.hpp>`

```cpp
struct AppMetadata { std::wstring name, displayName, description,
                     version, author, vendor; int sdkVersion; };

class IApplication {
    virtual bool Initialize(PluginContext& context) = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) {}
    virtual void Render() {}
    virtual const AppMetadata& GetMetadata() = 0;
};
```

---

## Plugin Entry Point

**Header:** `<DragonOS/PluginAPI.hpp>`

```cpp
#define DRAGONOS_PLUGIN_EXPORT __declspec(dllexport)
#define DRAGONOS_DECLARE_PLUGIN(AppClass) ...  // see PluginAPI.hpp
```

---

## Macro Index

| Macro                      | Header                        |
|----------------------------|-------------------------------|
| `DRAGONOS_SDK_VERSION`     | `<DragonOS/Version.hpp>`      |
| `DRAGONOS_PLUGIN_EXPORT`   | `<DragonOS/PluginAPI.hpp>`    |
| `DRAGONOS_DECLARE_PLUGIN` | `<DragonOS/PluginAPI.hpp>`    |
