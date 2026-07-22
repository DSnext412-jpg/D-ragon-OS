#pragma once

#include <ExtensionPoints/ExtensionPoint.hpp>

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace DragonOS::Graphics { class Renderer; }

namespace DragonOS::ExtensionPoints {

class TaskbarWidget : public IExtensionPoint {
public:
    explicit TaskbarWidget(std::wstring_view name, std::wstring_view pluginName) noexcept
        : m_name{ name }
        , m_pluginName{ pluginName }
    {
    }

    ExtensionPointType GetType() const noexcept override { return ExtensionPointType::TaskbarWidget; }
    std::wstring_view GetName() const noexcept override { return m_name; }
    std::wstring_view GetPluginName() const noexcept override { return m_pluginName; }

    virtual void Render(Graphics::Renderer& renderer, const Input::Bounds& area) noexcept = 0;
    virtual bool HitTest(float px, float py) const noexcept = 0;
    virtual void OnClick() noexcept = 0;

    virtual float GetWidth() const noexcept { return 32.0f; }
    virtual std::wstring_view GetTooltip() const noexcept { return L""; }

    Input::Bounds bounds{};

private:
    std::wstring m_name;
    std::wstring m_pluginName;
};

} // namespace DragonOS::ExtensionPoints
