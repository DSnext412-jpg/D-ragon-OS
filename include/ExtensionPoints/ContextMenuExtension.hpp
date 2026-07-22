#pragma once

#include <ExtensionPoints/ExtensionPoint.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::ExtensionPoints {

class ContextMenuExtension : public IExtensionPoint {
public:
    explicit ContextMenuExtension(std::wstring_view name, std::wstring_view pluginName) noexcept
        : m_name{ name }
        , m_pluginName{ pluginName }
    {
    }

    ExtensionPointType GetType() const noexcept override { return ExtensionPointType::ContextMenuItem; }
    std::wstring_view GetName() const noexcept override { return m_name; }
    std::wstring_view GetPluginName() const noexcept override { return m_pluginName; }

    virtual std::wstring_view GetDisplayName() const noexcept = 0;
    virtual std::vector<std::wstring> GetFileExtensions() const noexcept { return {}; }
    virtual bool AppliesToFolders() const noexcept { return false; }
    virtual bool AppliesToFiles() const noexcept { return false; }
    virtual bool AppliesToDesktop() const noexcept { return false; }

    virtual void OnActivate(std::wstring_view targetPath) noexcept = 0;

private:
    std::wstring m_name;
    std::wstring m_pluginName;
};

} // namespace DragonOS::ExtensionPoints
