#pragma once

#include <Plugins/PluginMetadata.hpp>

#include <DragonOS/Application.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace DragonOS::Plugins {

class PluginLoader final {
public:
    PluginLoader() noexcept = default;
    ~PluginLoader() noexcept { Shutdown(); }

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    struct LoadResult {
        bool success{ false };
        std::wstring errorMessage;
        dragonos::sdk::IApplication* app{ nullptr };
        PluginMetadata metadata;
    };

    LoadResult Load(std::wstring_view pluginPath) noexcept;
    bool Unload(dragonos::sdk::IApplication* app) noexcept;

    void Shutdown() noexcept;

private:
    struct LoadedModule {
        void* hModule{ nullptr };
        dragonos::sdk::IApplication* app{ nullptr };
        bool ownsApp{ false };
    };

    std::vector<LoadedModule> m_modules;
};

} // namespace DragonOS::Plugins
