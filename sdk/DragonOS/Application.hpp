#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace dragonos::sdk {

class PluginContext;

struct AppMetadata {
    std::wstring name;
    std::wstring displayName;
    std::wstring description;
    std::wstring version;
    std::wstring author;
    std::wstring vendor;
    int sdkVersion{ 0 };
};

class IApplication {
public:
    virtual ~IApplication() noexcept = default;

    virtual bool Initialize(PluginContext& context) noexcept = 0;
    virtual void Shutdown() noexcept = 0;

    virtual void Update(float deltaTime) noexcept { (void)deltaTime; }
    virtual void Render() noexcept {}

    virtual const AppMetadata& GetMetadata() const noexcept = 0;
};

} // namespace dragonos::sdk
