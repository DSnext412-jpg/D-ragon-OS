#include <Plugins/PluginRegistry.hpp>

namespace DragonOS::Plugins {

void PluginRegistry::Register(
    std::wstring name,
    dragonos::sdk::IApplication* app,
    PluginMetadata metadata) noexcept
{
    PluginEntry entry;
    entry.app = app;
    entry.metadata = std::move(metadata);
    entry.metadata.name = name;
    m_plugins[std::move(name)] = std::move(entry);
}

void PluginRegistry::Unregister(std::wstring_view name) noexcept
{
    m_plugins.erase(std::wstring{ name });
}

dragonos::sdk::IApplication* PluginRegistry::Find(std::wstring_view name) noexcept
{
    auto it = m_plugins.find(std::wstring{ name });
    return (it != m_plugins.end()) ? it->second.app : nullptr;
}

const PluginMetadata* PluginRegistry::FindMetadata(std::wstring_view name) const noexcept
{
    auto it = m_plugins.find(std::wstring{ name });
    return (it != m_plugins.end()) ? &it->second.metadata : nullptr;
}

std::vector<std::wstring> PluginRegistry::GetNames() const noexcept
{
    std::vector<std::wstring> names;
    names.reserve(m_plugins.size());
    for (const auto& [name, entry] : m_plugins)
    {
        names.push_back(name);
    }
    return names;
}

std::vector<const PluginMetadata*> PluginRegistry::GetAllMetadata() const noexcept
{
    std::vector<const PluginMetadata*> result;
    result.reserve(m_plugins.size());
    for (const auto& [name, entry] : m_plugins)
    {
        result.push_back(&entry.metadata);
    }
    return result;
}

} // namespace DragonOS::Plugins
