#pragma once

#include <Plugins/PluginLoader.hpp>
#include <Plugins/PluginRegistry.hpp>
#include <Plugins/PluginContext.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace DragonOS::Events          { class EventBus; }
namespace DragonOS::Notifications   { class NotificationManager; }
namespace DragonOS::ExtensionPoints { class ExtensionPointManager; }

namespace DragonOS::Plugins {

using PluginContext = dragonos::sdk::PluginContext;

class PluginManager final : public Engine::System {
public:
    PluginManager() noexcept = default;
    ~PluginManager() noexcept override { Shutdown(); }

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    PluginManager(PluginManager&&) = delete;
    PluginManager& operator=(PluginManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    bool LoadPlugin(std::wstring_view path) noexcept;
    bool UnloadPlugin(std::wstring_view name) noexcept;
    bool EnablePlugin(std::wstring_view name) noexcept;
    bool DisablePlugin(std::wstring_view name) noexcept;

    using PluginEventCallback = std::function<void(std::wstring_view pluginName)>;
    void SetOnPluginLoaded(PluginEventCallback cb) noexcept { m_onLoaded = std::move(cb); }
    void SetOnPluginUnloaded(PluginEventCallback cb) noexcept { m_onUnloaded = std::move(cb); }

    PluginRegistry& GetRegistry() noexcept { return m_registry; }
    const PluginRegistry& GetRegistry() const noexcept { return m_registry; }

    void SetExtensionPointManager(ExtensionPoints::ExtensionPointManager& epm) noexcept;

    void ScanPluginsDirectory(std::wstring_view directory) noexcept;

    void SetNotificationService(dragonos::sdk::INotificationService* s) noexcept { m_pNotificationService = s; }
    void SetConfigService(dragonos::sdk::IConfigService* s) noexcept { m_pConfigService = s; }
    void SetFileService(dragonos::sdk::IFileService* s) noexcept { m_pFileService = s; }
    void SetThemeService(dragonos::sdk::IThemeService* s) noexcept { m_pThemeService = s; }
    void SetInputService(dragonos::sdk::IInputService* s) noexcept { m_pInputService = s; }
    void SetResourceService(dragonos::sdk::IResourceService* s) noexcept { m_pResourceService = s; }
    void SetWindowService(dragonos::sdk::IWindowService* s) noexcept { m_pWindowService = s; }
    void SetMenuService(dragonos::sdk::IMenuService* s) noexcept { m_pMenuService = s; }
    void SetDialogService(dragonos::sdk::IDialogService* s) noexcept { m_pDialogService = s; }
    void SetEventBus(dragonos::sdk::IEventBus* s) noexcept { m_pEventBus = s; }

private:
    bool InitializePlugin(dragonos::sdk::IApplication* app, PluginMetadata metadata) noexcept;
    bool ValidatePlugin(const PluginMetadata& metadata) const noexcept;

    PluginLoader m_loader;
    PluginRegistry m_registry;
    std::vector<std::unique_ptr<PluginContext>> m_contexts;

    ExtensionPoints::ExtensionPointManager* m_pExtensionPointMgr{ nullptr };
    PluginEventCallback m_onLoaded;
    PluginEventCallback m_onUnloaded;

    dragonos::sdk::INotificationService* m_pNotificationService{ nullptr };
    dragonos::sdk::IConfigService*       m_pConfigService{ nullptr };
    dragonos::sdk::IFileService*         m_pFileService{ nullptr };
    dragonos::sdk::IThemeService*        m_pThemeService{ nullptr };
    dragonos::sdk::IInputService*        m_pInputService{ nullptr };
    dragonos::sdk::IResourceService*     m_pResourceService{ nullptr };
    dragonos::sdk::IWindowService*       m_pWindowService{ nullptr };
    dragonos::sdk::IMenuService*         m_pMenuService{ nullptr };
    dragonos::sdk::IDialogService*       m_pDialogService{ nullptr };
    dragonos::sdk::IEventBus*            m_pEventBus{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Plugins
