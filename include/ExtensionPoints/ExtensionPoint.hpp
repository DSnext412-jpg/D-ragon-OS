#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::ExtensionPoints {

enum class ExtensionPointType : uint8_t {
    StartMenuEntry,
    ContextMenuItem,
    TaskbarWidget,
    SettingsPage,
    SearchProvider,
    NotificationProvider,
    TerminalCommand,
};

class IExtensionPoint {
public:
    virtual ~IExtensionPoint() noexcept = default;
    virtual ExtensionPointType GetType() const noexcept = 0;
    virtual std::wstring_view GetName() const noexcept = 0;
    virtual std::wstring_view GetPluginName() const noexcept = 0;
};

class ExtensionPointManager final {
public:
    ExtensionPointManager() noexcept = default;
    ~ExtensionPointManager() noexcept = default;

    void Register(ExtensionPointType type, std::unique_ptr<IExtensionPoint> point) noexcept;
    void Unregister(std::wstring_view name) noexcept;
    void UnregisterPlugin(std::wstring_view pluginName) noexcept;

    std::vector<IExtensionPoint*> GetByType(ExtensionPointType type) const noexcept;
    IExtensionPoint* Find(std::wstring_view name) const noexcept;

    size_t GetCount() const noexcept { return m_points.size(); }
    size_t GetCountByType(ExtensionPointType type) const noexcept;

    void Clear() noexcept;

private:
    struct ExtensionEntry {
        std::unique_ptr<IExtensionPoint> point;
        ExtensionPointType type;
    };

    std::vector<ExtensionEntry> m_points;
};

} // namespace DragonOS::ExtensionPoints
