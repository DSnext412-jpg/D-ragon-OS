#pragma once

#include <DragonOS/Notification.hpp>
#include <DragonOS/Config.hpp>
#include <DragonOS/File.hpp>
#include <DragonOS/Theme.hpp>
#include <DragonOS/Input.hpp>
#include <DragonOS/Resource.hpp>
#include <DragonOS/Window.hpp>
#include <DragonOS/Menu.hpp>
#include <DragonOS/Dialog.hpp>
#include <DragonOS/Events.hpp>

#include <Plugins/PluginMetadata.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace DragonOS::Events { class EventBus; }
namespace DragonOS::Notifications { class NotificationManager; }

namespace dragonos::sdk {

using PluginMetadata = DragonOS::Plugins::PluginMetadata;

class PluginContext final {
public:
    explicit PluginContext(PluginMetadata metadata) noexcept
        : m_metadata{ std::move(metadata) }
    {
    }

    const PluginMetadata& GetMetadata() const noexcept { return m_metadata; }

    INotificationService* GetNotificationService() const noexcept { return m_pNotificationService; }
    IConfigService*       GetConfigService()       const noexcept { return m_pConfigService; }
    IFileService*         GetFileService()         const noexcept { return m_pFileService; }
    IThemeService*        GetThemeService()        const noexcept { return m_pThemeService; }
    IInputService*        GetInputService()        const noexcept { return m_pInputService; }
    IResourceService*     GetResourceService()     const noexcept { return m_pResourceService; }
    IWindowService*       GetWindowService()       const noexcept { return m_pWindowService; }
    IMenuService*         GetMenuService()         const noexcept { return m_pMenuService; }
    IDialogService*       GetDialogService()       const noexcept { return m_pDialogService; }
    IEventBus*            GetEventBus()            const noexcept { return m_pEventBus; }

    void SetNotificationService(INotificationService* s) noexcept { m_pNotificationService = s; }
    void SetConfigService(IConfigService* s) noexcept { m_pConfigService = s; }
    void SetFileService(IFileService* s) noexcept { m_pFileService = s; }
    void SetThemeService(IThemeService* s) noexcept { m_pThemeService = s; }
    void SetInputService(IInputService* s) noexcept { m_pInputService = s; }
    void SetResourceService(IResourceService* s) noexcept { m_pResourceService = s; }
    void SetWindowService(IWindowService* s) noexcept { m_pWindowService = s; }
    void SetMenuService(IMenuService* s) noexcept { m_pMenuService = s; }
    void SetDialogService(IDialogService* s) noexcept { m_pDialogService = s; }
    void SetEventBus(IEventBus* s) noexcept { m_pEventBus = s; }

private:
    PluginMetadata m_metadata;

    INotificationService* m_pNotificationService{ nullptr };
    IConfigService*       m_pConfigService{ nullptr };
    IFileService*         m_pFileService{ nullptr };
    IThemeService*        m_pThemeService{ nullptr };
    IInputService*        m_pInputService{ nullptr };
    IResourceService*     m_pResourceService{ nullptr };
    IWindowService*       m_pWindowService{ nullptr };
    IMenuService*         m_pMenuService{ nullptr };
    IDialogService*       m_pDialogService{ nullptr };
    IEventBus*            m_pEventBus{ nullptr };
};

} // namespace dragonos::sdk
