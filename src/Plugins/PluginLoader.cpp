#include <Plugins/PluginLoader.hpp>

#include <DragonOS/Version.hpp>

#include <Windows.h>

#include <cstdio>
#include <string>

namespace DragonOS::Plugins {

using CreatePluginFunc = dragonos::sdk::IApplication* (*)();
using DestroyPluginFunc = void (*)(dragonos::sdk::IApplication*);
using GetSDKVersionFunc = int (*)();

PluginLoader::LoadResult PluginLoader::Load(std::wstring_view pluginPath) noexcept
{
    LoadResult result;

    HMODULE hModule = ::LoadLibraryW(std::wstring{ pluginPath }.c_str());
    if (!hModule)
    {
        result.errorMessage = L"Failed to load DLL: ";
        result.errorMessage += pluginPath;
        return result;
    }

    auto getVersion = reinterpret_cast<GetSDKVersionFunc>(
        ::GetProcAddress(hModule, "GetPluginSDKVersion"));
    if (!getVersion)
    {
        result.errorMessage = L"Plugin missing GetPluginSDKVersion export";
        ::FreeLibrary(hModule);
        return result;
    }

    const int sdkVer = getVersion();
    if (!dragonos::sdk::SDKVersion::IsCompatible(sdkVer))
    {
        wchar_t buf[128];
        ::swprintf_s(buf, L"Incompatible SDK version %d (host supports %d)",
                     sdkVer, DRAGONOS_SDK_VERSION);
        result.errorMessage = buf;
        ::FreeLibrary(hModule);
        return result;
    }

    auto createPlugin = reinterpret_cast<CreatePluginFunc>(
        ::GetProcAddress(hModule, "CreatePlugin"));
    if (!createPlugin)
    {
        result.errorMessage = L"Plugin missing CreatePlugin export";
        ::FreeLibrary(hModule);
        return result;
    }

    auto destroyPlugin = reinterpret_cast<DestroyPluginFunc>(
        ::GetProcAddress(hModule, "DestroyPlugin"));
    if (!destroyPlugin)
    {
        result.errorMessage = L"Plugin missing DestroyPlugin export";
        ::FreeLibrary(hModule);
        return result;
    }

    dragonos::sdk::IApplication* app = createPlugin();
    if (!app)
    {
        result.errorMessage = L"CreatePlugin returned null";
        ::FreeLibrary(hModule);
        return result;
    }

    result.success = true;
    result.app = app;

    result.metadata.sdkVersion = sdkVer;
    result.metadata.pluginPath = pluginPath;
    result.metadata.name = app->GetMetadata().name;
    result.metadata.displayName = app->GetMetadata().displayName;
    result.metadata.description = app->GetMetadata().description;
    result.metadata.version = app->GetMetadata().version;
    result.metadata.author = app->GetMetadata().author;
    result.metadata.vendor = app->GetMetadata().vendor;

    LoadedModule mod;
    mod.hModule = hModule;
    mod.app = app;
    mod.ownsApp = true;
    m_modules.push_back(std::move(mod));

    return result;
}

bool PluginLoader::Unload(dragonos::sdk::IApplication* app) noexcept
{
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
    {
        if (it->app == app)
        {
            auto destroyPlugin = reinterpret_cast<DestroyPluginFunc>(
                ::GetProcAddress(static_cast<HMODULE>(it->hModule), "DestroyPlugin"));
            if (destroyPlugin && it->ownsApp)
            {
                destroyPlugin(it->app);
            }
            ::FreeLibrary(static_cast<HMODULE>(it->hModule));
            m_modules.erase(it);
            return true;
        }
    }
    return false;
}

void PluginLoader::Shutdown() noexcept
{
    for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
    {
        if (it->hModule)
        {
            auto destroyPlugin = reinterpret_cast<DestroyPluginFunc>(
                ::GetProcAddress(static_cast<HMODULE>(it->hModule), "DestroyPlugin"));
            if (destroyPlugin && it->ownsApp)
            {
                destroyPlugin(it->app);
            }
            ::FreeLibrary(static_cast<HMODULE>(it->hModule));
        }
    }
    m_modules.clear();
}

} // namespace DragonOS::Plugins
