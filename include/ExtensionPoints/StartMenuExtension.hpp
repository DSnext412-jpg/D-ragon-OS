#pragma once

#include <ExtensionPoints/ExtensionPoint.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace DragonOS::ExtensionPoints {

class StartMenuExtension : public IExtensionPoint {
public:
    explicit StartMenuExtension(std::wstring_view name, std::wstring_view pluginName) noexcept
        : m_name{ name }
        , m_pluginName{ pluginName }
    {
    }

    ExtensionPointType GetType() const noexcept override { return ExtensionPointType::StartMenuEntry; }
    std::wstring_view GetName() const noexcept override { return m_name; }
    std::wstring_view GetPluginName() const noexcept override { return m_pluginName; }

    virtual std::wstring_view GetDisplayName() const noexcept = 0;
    virtual std::wstring_view GetIcon() const noexcept { return L""; }

    using ActivateCallback = std::function<void()>;
    virtual void OnActivate() noexcept = 0;

private:
    std::wstring m_name;
    std::wstring m_pluginName;
};

} // namespace DragonOS::ExtensionPoints
