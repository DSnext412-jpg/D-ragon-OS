#pragma once

#include <ExtensionPoints/ExtensionPoint.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace DragonOS::Graphics { class Renderer; }

namespace DragonOS::ExtensionPoints {

class SettingsPage : public IExtensionPoint {
public:
    explicit SettingsPage(std::wstring_view name, std::wstring_view pluginName) noexcept
        : m_name{ name }
        , m_pluginName{ pluginName }
    {
    }

    ExtensionPointType GetType() const noexcept override { return ExtensionPointType::SettingsPage; }
    std::wstring_view GetName() const noexcept override { return m_name; }
    std::wstring_view GetPluginName() const noexcept override { return m_pluginName; }

    virtual std::wstring_view GetDisplayName() const noexcept = 0;
    virtual std::wstring_view GetCategory() const noexcept { return L"Plugins"; }

    virtual void Render(Graphics::Renderer& renderer, float x, float y, float width, float height) noexcept = 0;
    virtual void Update(float deltaTime) noexcept { (void)deltaTime; }
    virtual void OnActivate() noexcept {}
    virtual void OnDeactivate() noexcept {}

private:
    std::wstring m_name;
    std::wstring m_pluginName;
};

} // namespace DragonOS::ExtensionPoints
