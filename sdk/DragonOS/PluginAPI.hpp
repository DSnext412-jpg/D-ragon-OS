#pragma once

#include <DragonOS/Application.hpp>

extern "C" {
    using CreatePluginFunc = dragonos::sdk::IApplication* (*)();
    using DestroyPluginFunc = void (*)(dragonos::sdk::IApplication*);
}

#define DRAGONOS_PLUGIN_EXPORT extern "C" __declspec(dllexport)

#define DRAGONOS_DECLARE_PLUGIN(AppClass) \
    DRAGONOS_PLUGIN_EXPORT dragonos::sdk::IApplication* CreatePlugin() \
    { \
        return new AppClass(); \
    } \
    DRAGONOS_PLUGIN_EXPORT void DestroyPlugin(dragonos::sdk::IApplication* app) \
    { \
        delete app; \
    } \
    DRAGONOS_PLUGIN_EXPORT int GetPluginSDKVersion() \
    { \
        return DRAGONOS_SDK_VERSION; \
    }
