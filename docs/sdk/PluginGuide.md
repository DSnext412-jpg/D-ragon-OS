# DragonOS Plugin Guide

## Architecture

A DragonOS plugin is a Windows DLL that exports three C-ABI functions:

- `CreatePlugin` — returns an `IApplication*` instance
- `DestroyPlugin` — destroys the instance
- `GetPluginSDKVersion` — returns the SDK version the plugin was compiled against

The `DRAGONOS_DECLARE_PLUGIN(AppClass)` macro implements all three.

## Lifecycle

```
Load DLL
  → GetPluginSDKVersion() — version compatibility check
  → CreatePlugin()        — instantiate IApplication
  → Initialize(ctx)       — plugin receives service pointers
  → Update(dt)            — called every frame
  → Shutdown()            — called on plugin unload
  → DestroyPlugin()       — free the instance
Unload DLL
```

## Available Services

All services are accessed through `PluginContext` during `Initialize()`:

| Service             | Getter                          | Purpose                          |
|---------------------|---------------------------------|----------------------------------|
| Notifications       | `GetNotificationService()`      | Show/dismiss toast notifications |
| Configuration       | `GetConfigService()`            | Persistent key-value storage     |
| File System         | `GetFileService()`              | Directory/file operations        |
| Theme               | `GetThemeService()`             | Query colours and dark mode      |
| Input               | `GetInputService()`             | Keyboard and mouse state         |
| Resources           | `GetResourceService()`          | Load binary/text data files      |
| Window Management   | `GetWindowService()`            | Create/manage application windows|
| Menu                | `GetMenuService()`              | Context menus                    |
| Dialog              | `GetDialogService()`            | Message boxes and file dialogs   |
| Event Bus           | `GetEventBus()`                 | Publish/subscribe events         |

## Logging

```cpp
auto& log = dragonos::sdk::Logger::Get();
log.Info(L"Hello");
log.Warning(L"Something suspicious");
log.Error(L"Something broke");
```

## Notifications

```cpp
auto* notif = ctx.GetNotificationService();
dragonos::sdk::NotificationData n;
n.title = L"Update Available";
n.message = L"Version 2.0 is ready to install.";
n.severity = dragonos::sdk::NotificationSeverity::Information;
notif->Show(n);
```

## Configuration

```cpp
auto* config = ctx.GetConfigService();
auto& sec = config->GetSection(L"MyPlugin");
sec.Set(L"theme", L"dark");
std::wstring theme = sec.Get(L"theme", L"light");
sec.Set(L"volume", L"75");
int vol = sec.GetInt(L"volume", 50);
config->Save();
```

## Windows

```cpp
auto* wm = ctx.GetWindowService();
dragonos::sdk::WindowCreateParams p;
p.title = L"My Window";
p.width = 640; p.height = 480;
p.resizable = true;

auto* wnd = wm->Create(p);
wnd->SetOnClose([](uint64_t id) { /* cleanup */ });
```

## Events

```cpp
auto* bus = ctx.GetEventBus();

// Subscribe
auto id = bus->Subscribe(
    dragonos::sdk::EventType::ThemeChanged,
    [](const dragonos::sdk::Event& e) { /* react */ });

// Publish
dragonos::sdk::Event e;
e.type = dragonos::sdk::EventType::Custom;
e.sourceName = L"MyPlugin";
bus->Publish(e);
```

## Extension Points

Plugins can extend DragonOS UI through extension points:

- **Start Menu** — register a tile/entry
- **Context Menu** — add items to Explorer file/folder menus
- **Taskbar Widget** — add a notification area widget
- **Settings Page** — add a page to the Settings app

These are registered via `ExtensionPointManager` during initialization.

## SDK Versioning

```cpp
DRAGONOS_SDK_VERSION_MAJOR  // 1
DRAGONOS_SDK_VERSION_MINOR  // 0
DRAGONOS_SDK_VERSION_PATCH  // 0
DRAGONOS_SDK_VERSION        // 10000 (MAJOR*10000 + MINOR*100 + PATCH)
```

The SDK checks `IsCompatible(pluginSdkVersion)` — plugins must be within
the same major version.
