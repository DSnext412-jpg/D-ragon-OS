#include <Plugins/PluginManager.hpp>

#include <DragonOS/Version.hpp>

#include <ExtensionPoints/ExtensionPoint.hpp>
#include <ExtensionPoints/StartMenuExtension.hpp>
#include <Security/PermissionManager.hpp>
#include <Security/SecurityTypes.hpp>

#include <Engine/EngineContext.hpp>
#include <Engine/SystemManager.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

namespace DragonOS::Plugins {

bool PluginManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void PluginManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    const auto names = m_registry.GetNames();
    for (const auto& name : names)
    {
        auto* app = m_registry.Find(name);
        if (app)
        {
            app->Shutdown();
        }
    }

    m_loader.Shutdown();
    m_contexts.clear();
    m_initialized = false;
}

void PluginManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    for (auto& ctx : m_contexts)
    {
        auto* app = m_registry.Find(ctx->GetMetadata().name);
        if (app)
        {
            app->Update(deltaTime);
        }
    }
}

void PluginManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void PluginManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

bool PluginManager::LoadPlugin(std::wstring_view path) noexcept
{
    if (!m_initialized) { return false; }

    const auto result = m_loader.Load(path);
    if (!result.success)
    {
        return false;
    }

    if (!InitializePlugin(result.app, result.metadata))
    {
        m_loader.Unload(result.app);
        return false;
    }

    if (m_onLoaded)
    {
        m_onLoaded(result.metadata.name);
    }

    return true;
}

bool PluginManager::UnloadPlugin(std::wstring_view name) noexcept
{
    auto* app = m_registry.Find(name);
    if (!app) { return false; }

    app->Shutdown();

    if (m_pExtensionPointMgr)
    {
        m_pExtensionPointMgr->UnregisterPlugin(name);
    }

    m_loader.Unload(app);
    m_registry.Unregister(name);

    m_contexts.erase(
        std::remove_if(m_contexts.begin(), m_contexts.end(),
            [&](const auto& ctx) { return ctx->GetMetadata().name == name; }),
        m_contexts.end());

    if (m_onUnloaded)
    {
        m_onUnloaded(name);
    }

    return true;
}

bool PluginManager::EnablePlugin(std::wstring_view name) noexcept
{
    auto* meta = const_cast<PluginMetadata*>(m_registry.FindMetadata(name));
    if (!meta) { return false; }
    meta->enabled = true;
    return true;
}

bool PluginManager::DisablePlugin(std::wstring_view name) noexcept
{
    auto* meta = const_cast<PluginMetadata*>(m_registry.FindMetadata(name));
    if (!meta) { return false; }
    meta->enabled = false;
    return true;
}

void PluginManager::SetExtensionPointManager(
    ExtensionPoints::ExtensionPointManager& epm) noexcept
{
    m_pExtensionPointMgr = &epm;
}

void PluginManager::ScanPluginsDirectory(std::wstring_view directory) noexcept
{
    if (!m_initialized) { return; }

    namespace fs = std::filesystem;
    fs::path dir{ directory.data() };

    if (!fs::exists(dir) || !fs::is_directory(dir)) { return; }

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file() &&
            entry.path().extension() == L".dll" &&
            entry.path().filename().wstring().find(L"Plugin") != std::wstring::npos)
        {
            LoadPlugin(entry.path().wstring());
        }
    }
}

bool PluginManager::InitializePlugin(
    dragonos::sdk::IApplication* app,
    PluginMetadata metadata) noexcept
{
    auto ctx = std::make_unique<PluginContext>(metadata);

    ctx->SetNotificationService(m_pNotificationService);
    ctx->SetConfigService(m_pConfigService);
    ctx->SetFileService(m_pFileService);
    ctx->SetThemeService(m_pThemeService);
    ctx->SetInputService(m_pInputService);
    ctx->SetResourceService(m_pResourceService);
    ctx->SetWindowService(m_pWindowService);
    ctx->SetMenuService(m_pMenuService);
    ctx->SetDialogService(m_pDialogService);
    ctx->SetEventBus(m_pEventBus);

    auto* metaPtr = ctx.get();
    m_registry.Register(metadata.name, app, std::move(metadata));
    m_contexts.push_back(std::move(ctx));

    if (!app->Initialize(*metaPtr))
    {
        return false;
    }

    return true;
}

bool PluginManager::ValidatePlugin(const PluginMetadata& metadata) const noexcept
{
    if (!dragonos::sdk::SDKVersion::IsCompatible(metadata.sdkVersion))
    {
        return false;
    }

    for (const auto& dep : metadata.dependencies)
    {
        const PluginMetadata* depMeta = m_registry.FindMetadata(dep.name);
        if (!depMeta) { return false; }
    }

    // Check permissions against current user's security context
    if (m_pPermissionMgr)
    {
        using Security::Permission;
        using Security::HasPermission;

        // Map plugin permissions to security permissions
        Permission requiredPerms = Permission::None;

        if (HasPermission(metadata.permissions, PluginPermission::FileAccess))
            requiredPerms = requiredPerms | Permission::FileRead | Permission::FileWrite;

        if (HasPermission(metadata.permissions, PluginPermission::Notifications))
            requiredPerms = requiredPerms | Permission::AppLaunch;

        if (HasPermission(metadata.permissions, PluginPermission::WindowManagement))
            requiredPerms = requiredPerms | Permission::AppLaunch;

        if (HasPermission(metadata.permissions, PluginPermission::Configuration))
            requiredPerms = requiredPerms | Permission::SettingsWrite;

        if (HasPermission(metadata.permissions, PluginPermission::Network))
            requiredPerms = requiredPerms | Permission::AppLaunch;

        if (requiredPerms != Permission::None)
        {
            if (!m_pPermissionMgr->CheckPermission(requiredPerms))
            {
                return false;
            }
        }
    }

    return true;
}

} // namespace DragonOS::Plugins
