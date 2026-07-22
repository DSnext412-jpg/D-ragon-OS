#pragma once

#include <Plugins/PluginMetadata.hpp>

#include <DragonOS/Application.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Plugins {

class PluginRegistry final {
public:
    PluginRegistry() noexcept = default;
    ~PluginRegistry() noexcept = default;

    PluginRegistry(const PluginRegistry&) = delete;
    PluginRegistry& operator=(const PluginRegistry&) = delete;

    void Register(std::wstring name, dragonos::sdk::IApplication* app, PluginMetadata metadata) noexcept;
    void Unregister(std::wstring_view name) noexcept;

    dragonos::sdk::IApplication* Find(std::wstring_view name) noexcept;
    const PluginMetadata* FindMetadata(std::wstring_view name) const noexcept;

    std::vector<std::wstring> GetNames() const noexcept;
    std::vector<const PluginMetadata*> GetAllMetadata() const noexcept;
    size_t GetCount() const noexcept { return m_plugins.size(); }

private:
    struct PluginEntry {
        dragonos::sdk::IApplication* app{ nullptr };
        PluginMetadata metadata;
        bool ownsApp{ false };
    };

    std::unordered_map<std::wstring, PluginEntry> m_plugins;
};

} // namespace DragonOS::Plugins
