#pragma once

#include <cstdint>
#include <string>

namespace DragonOS::AppRuntime {

enum class ApplicationState : uint8_t {
    Running,
    Suspended,
    Terminated
};

struct ApplicationMetadata final {
    uint64_t    appId{ 0 };
    uint64_t    registryId{ 0 };
    std::wstring name;
    std::wstring displayName;
    ApplicationState state{ ApplicationState::Running };
    uint64_t    launchTime{ 0 };
    bool        isSystem{ false };
};

} // namespace DragonOS::AppRuntime
