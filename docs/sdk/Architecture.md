# DragonOS SDK Architecture

## Overview

The DragonOS SDK enables third-party developers to create applications
("plugins") that run inside the DragonOS desktop environment. The
architecture follows a layered design:

```
┌─────────────────────────────────────────────────────────┐
│                   Plugin (DLL)                           │
│  ┌────────────────────────────────────────────────────┐  │
│  │  IApplication (Initialize / Update / Shutdown)     │  │
│  └────────────┬───────────────────────────────────────┘  │
│               │  uses                                    │
│  ┌────────────▼───────────────────────────────────────┐  │
│  │  dragonos::sdk::* service interfaces               │  │
│  │  (INotificationService, IWindowService, etc.)      │  │
│  └────────────┬───────────────────────────────────────┘  │
└───────────────┼──────────────────────────────────────────┘
                │  C ABI (CreatePlugin / DestroyPlugin)
┌───────────────▼──────────────────────────────────────────┐
│                    DragonOS Host                          │
│  ┌─────────────────────────────────────────────────────┐ │
│  │  PluginManager (load/unload lifecycle)              │ │
│  │  ExtensionPointManager (start menu, context menu)   │ │
│  │  EventBus (pub/sub event routing)                   │ │
│  └────────────┬────────────────────────────────────────┘ │
│               │  adapters                                │
│  ┌────────────▼────────────────────────────────────────┐ │
│  │  SDK Adapters (translate SDK → internal API)        │ │
│  │  WindowServiceAdapter, ThemeServiceAdapter, ...     │ │
│  └────────────┬────────────────────────────────────────┘ │
│               │  internal API                            │
│  ┌────────────▼────────────────────────────────────────┐ │
│  │  DragonOS Internal Systems                          │ │
│  │  WindowManager, ThemeManager, NotificationMgr, ...  │ │
│  └─────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────┘
```

## Key Components

### SDK Headers (`sdk/DragonOS/`)
Self-contained, header-only C++20 interfaces. No dependency on
internal DragonOS headers. Every interface is abstract (pure virtual)
so plugins only link against the SDK headers, never internal code.

### Plugin Host (`include/Plugins/`)
Internal headers that are NOT exposed to plugins:
- **PluginManager** — Engine::System that orchestrates loading,
  unloading, and per-frame update of all plugins.
- **PluginLoader** — Win32 DLL loading with `CreatePlugin`/
  `DestroyPlugin` C-ABI calls.
- **PluginRegistry** — Map of plugin name → IApplication* + metadata.
- **PluginContext** — Provides service pointers to each plugin
  during initialization.

### SDK Adapters (`src/SDK/`)
Bridge between the public SDK interfaces and the internal DragonOS
systems. Each adapter implements one `dragonos::sdk::I*` interface
by wrapping a DragonOS internal system (e.g., `NotificationServiceAdapter`
wraps `NotificationManager`).

### Event Bus
- `Events::EventBus` — internal pub/sub with priority ordering,
  synchronous and asynchronous publishing.
- `EventBusAdapter` — exposes the bus through the SDK `IEventBus`
  interface to plugins.

### Extension Points
- `ExtensionPointManager` — register/unregister plugin extensions
  by type (StartMenu, ContextMenu, TaskbarWidget, SettingsPage).
- Used by DragonOS UI systems (StartMenu, Explorer) to query
  plugin-provided extensions at runtime.

## Data Flow

1. **Engine::Initialize()** registers PluginSystem (wraps PluginManager,
   EventBus, ExtensionPointManager, all SDK adapters).
2. On startup, PluginManager scans `<DragonOS>/plugins/` directory.
3. For each DLL, PluginLoader calls `GetPluginSDKVersion` → version
   check → `CreatePlugin` → `Initialize(ctx)`.
4. PluginContext provides all service pointers to the plugin.
5. Per frame, PluginManager::Update calls each active plugin's Update().
6. Plugins communicate with DragonOS exclusively through SDK service
   interfaces (never directly with internal systems).

## Versioning

SDK version is `MAJOR * 10000 + MINOR * 100 + PATCH`. Breaking changes
increment MAJOR. Plugins compiled against a different MAJOR are rejected
at load time.
