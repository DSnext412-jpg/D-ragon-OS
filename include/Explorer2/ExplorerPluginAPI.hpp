// ============================================================================
//  ExplorerPluginAPI.hpp — Public extension SDK for Explorer 2.0 (Part 12).
//
//  Plugins implement IExplorerPlugin and are registered with
//  ExplorerSystem::RegisterPlugin().  A plugin may contribute:
//    - context-menu items   (IExplorerContextMenuExtension)
//    - preview renderers    (PreviewPane::IPreviewProvider)
//    - "Open with" handlers (keyed by file extension)
//
//  Lifetime: plugins are owned by the registry (unique_ptr); raw pointers
//  handed out through IExplorerPluginHost remain valid until Shutdown.
// ============================================================================

#pragma once

#include <Explorer2/ContextMenuService.hpp>
#include <Explorer2/PreviewPane.hpp>

#include <functional>
#include <memory>
#include <new>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

class IExplorerPlugin;

class IExplorerPluginHost {
public:
    virtual ~IExplorerPluginHost() = default;

    /// Registers a context-menu extension owned by @p owner.
    virtual void AddContextMenuExtension(
        IExplorerPlugin* owner, IExplorerContextMenuExtension* extension) = 0;

    /// Registers a preview provider owned by @p owner.
    virtual void AddPreviewProvider(
        IExplorerPlugin* owner, IPreviewProvider* provider) = 0;

    /// Registers an "Open with" handler for @p dotExtension (e.g. L".md").
    virtual void AddOpenWithHandler(
        IExplorerPlugin* owner,
        const std::wstring& dotExtension,
        const std::wstring& handlerLabel,
        std::function<void(const FileSystem::FileEntry&)> action) = 0;

    /// Surfaces a live-region announcement (screen readers).
    virtual void Announce(const std::wstring& text) = 0;
};

class IExplorerPlugin {
public:
    virtual ~IExplorerPlugin() = default;

    [[nodiscard]] virtual const wchar_t* GetName() const noexcept = 0;

    /// Called once after registration; keep host pointers for later use.
    virtual void OnRegistered(IExplorerPluginHost& host) = 0;

    /// Called before the registry destroys the plugin.
    virtual void OnUnregistered() {}
};

} // namespace DragonOS::Explorer2

/// Convenience macro for shipping a plugin from a translation unit.
#define DRAGONOS_EXPLORER_EXPORT_PLUGIN(PluginClass)                          \
    extern "C" __declspec(dllexport) DragonOS::Explorer2::IExplorerPlugin*    \
    DragonOSExplorerCreatePlugin()                                            \
    {                                                                         \
        return new (std::nothrow) PluginClass();                              \
    }
