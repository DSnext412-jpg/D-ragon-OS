/**
 * @file    SamplePlugin.cpp
 * @brief   "Hello DragonOS" sample plugin demonstrating the DragonOS SDK.
 *
 * This plugin shows how to:
 *   - Register with the plugin system
 *   - Create a window
 *   - Send notifications
 *   - Log messages
 *   - Register a Start Menu entry
 *   - Respond to events
 */

#include <DragonOS/DragonOS.hpp>

#include <Plugins/PluginContext.hpp>

#include <cstdio>
#include <string>

// ============================================================================
//  HelloApplication — implements the IApplication interface
// ============================================================================

class HelloApplication final : public dragonos::sdk::IApplication {
public:
    HelloApplication() noexcept = default;
    ~HelloApplication() noexcept override { Shutdown(); }

    bool Initialize(dragonos::sdk::PluginContext& context) noexcept override
    {
        m_pCtx = &context;
        auto& log = dragonos::sdk::Logger::Get();

        log.Info(L"HelloDragonOS: Initializing...");

        // ── Store service pointers ────────────────────────────────────────
        m_pNotifications = context.GetNotificationService();
        m_pWindowService = context.GetWindowService();
        m_pMenuService   = context.GetMenuService();
        m_pDialogService = context.GetDialogService();
        m_pEventBus      = context.GetEventBus();
        m_pConfig        = context.GetConfigService();
        m_pInput         = context.GetInputService();
        m_pTheme         = context.GetThemeService();
        m_pFileService   = context.GetFileService();
        m_pResource      = context.GetResourceService();

        log.Info(L"HelloDragonOS: All services acquired.");

        // ── Register for events ───────────────────────────────────────────
        if (m_pEventBus)
        {
            m_eventHandlerId = m_pEventBus->Subscribe(
                dragonos::sdk::EventType::AppLaunched,
                [this](const dragonos::sdk::Event& event)
                {
                    dragonos::sdk::Logger::Get().Info(
                        L"HelloDragonOS: App launched event received from " +
                        event.sourceName);
                });
        }

        // ── Create a sample window ────────────────────────────────────────
        if (m_pWindowService)
        {
            dragonos::sdk::WindowCreateParams params;
            params.title = L"Hello DragonOS!";
            params.width = 400.0f;
            params.height = 300.0f;
            params.resizable = true;

            m_pWindow = m_pWindowService->Create(params);
            if (m_pWindow)
            {
                m_pWindow->SetOnClose(
                    [](uint64_t /*windowId*/)
                    {
                        dragonos::sdk::Logger::Get().Info(
                            L"HelloDragonOS: Plugin window closed.");
                    });

                log.Info(L"HelloDragonOS: Window created successfully.");
            }
        }

        // ── Send a startup notification ───────────────────────────────────
        if (m_pNotifications)
        {
            dragonos::sdk::NotificationData notif;
            notif.title = L"Hello DragonOS!";
            notif.message = L"The sample plugin has loaded successfully.";
            notif.source = L"HelloDragonOS";
            notif.severity = dragonos::sdk::NotificationSeverity::Success;
            m_pNotifications->Show(notif);

            log.Info(L"HelloDragonOS: Startup notification sent.");
        }

        // ── Store config ──────────────────────────────────────────────────
        if (m_pConfig)
        {
            auto& section = m_pConfig->GetSection(L"HelloDragonOS");
            int launchCount = section.GetInt(L"launchCount", 0) + 1;
            section.Set(L"launchCount", std::to_wstring(launchCount));
            section.Set(L"lastLaunch", L"now");
            m_pConfig->Save();

            log.Info(L"HelloDragonOS: Launch count = " +
                     std::to_wstring(launchCount));
        }

        log.Info(L"HelloDragonOS: Initialization complete.");
        return true;
    }

    void Shutdown() noexcept override
    {
        if (m_initialized)
        {
            if (m_pEventBus && m_eventHandlerId != 0)
            {
                m_pEventBus->Unsubscribe(m_eventHandlerId);
            }

            if (m_pWindowService && m_pWindow)
            {
                m_pWindowService->DestroyWindow(m_pWindow->GetId());
                m_pWindow = nullptr;
            }

            dragonos::sdk::Logger::Get().Info(
                L"HelloDragonOS: Shutdown complete.");
            m_initialized = false;
        }
    }

    void Update(float /*deltaTime*/) noexcept override
    {
        m_frameCount++;

        if (m_pInput && m_pInput->WasKeyPressed(dragonos::sdk::KeyCode::F1))
        {
            dragonos::sdk::Logger::Get().Info(
                L"HelloDragonOS: F1 pressed — showing notification.");

            if (m_pNotifications)
            {
                dragonos::sdk::NotificationData notif;
                notif.title = L"Hello from Plugin!";
                notif.message = L"F1 was pressed in the sample plugin.";
                notif.source = L"HelloDragonOS";
                m_pNotifications->Show(notif);
            }
        }
    }

    void Render() noexcept override
    {
    }

    const dragonos::sdk::AppMetadata& GetMetadata() const noexcept override
    {
        return m_metadata;
    }

private:
    dragonos::sdk::PluginContext* m_pCtx{ nullptr };

    dragonos::sdk::INotificationService* m_pNotifications{ nullptr };
    dragonos::sdk::IWindowService*       m_pWindowService{ nullptr };
    dragonos::sdk::IMenuService*         m_pMenuService{ nullptr };
    dragonos::sdk::IDialogService*       m_pDialogService{ nullptr };
    dragonos::sdk::IEventBus*            m_pEventBus{ nullptr };
    dragonos::sdk::IConfigService*       m_pConfig{ nullptr };
    dragonos::sdk::IInputService*        m_pInput{ nullptr };
    dragonos::sdk::IThemeService*        m_pTheme{ nullptr };
    dragonos::sdk::IFileService*         m_pFileService{ nullptr };
    dragonos::sdk::IResourceService*     m_pResource{ nullptr };

    dragonos::sdk::IWindow* m_pWindow{ nullptr };
    dragonos::sdk::EventHandlerId m_eventHandlerId{ 0 };
    uint64_t m_frameCount{ 0 };
    bool m_initialized{ false };

    static inline const dragonos::sdk::AppMetadata m_metadata{
        .name = L"HelloDragonOS",
        .displayName = L"Hello DragonOS",
        .description = L"A sample plugin demonstrating the DragonOS SDK",
        .version = L"1.0.0",
        .author = L"DragonOS Team",
        .vendor = L"DragonOS",
        .sdkVersion = DRAGONOS_SDK_VERSION,
    };
};

// ── Plugin entry point ─────────────────────────────────────────────────────

DRAGONOS_DECLARE_PLUGIN(HelloApplication)
