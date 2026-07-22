#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Plugins {

struct PluginDependency {
    std::wstring name;
    int minVersion{ 0 };
    int maxVersion{ 0 };
};

enum class PluginPermission : uint8_t {
    None = 0,
    Notifications = 1 << 0,
    FileAccess = 1 << 1,
    WindowManagement = 1 << 2,
    InputEvents = 1 << 3,
    Network = 1 << 4,
    Configuration = 1 << 5,
    All = 0xFF,
};

inline PluginPermission operator|(PluginPermission a, PluginPermission b) noexcept
{
    return static_cast<PluginPermission>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline bool HasPermission(PluginPermission flags, PluginPermission check) noexcept
{
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(check)) != 0;
}

struct PluginMetadata final {
    std::wstring name;
    std::wstring displayName;
    std::wstring description;
    std::wstring version;
    std::wstring author;
    std::wstring vendor;
    std::wstring pluginPath;
    int sdkVersion{ 0 };
    bool enabled{ true };
    bool builtIn{ false };
    PluginPermission permissions{ PluginPermission::None };
    std::vector<PluginDependency> dependencies;
};

} // namespace DragonOS::Plugins
