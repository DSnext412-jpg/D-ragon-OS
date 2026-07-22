#pragma once

#include <cstdint>

namespace DragonOS::Services {

enum class ServiceState : uint8_t {
    Stopped,
    Starting,
    Running,
    Pausing,
    Paused,
    Resuming,
    Stopping,
    Error,
};

inline const wchar_t* ServiceStateToString(ServiceState state) noexcept
{
    switch (state)
    {
    case ServiceState::Stopped:  return L"Stopped";
    case ServiceState::Starting: return L"Starting";
    case ServiceState::Running:  return L"Running";
    case ServiceState::Pausing:  return L"Pausing";
    case ServiceState::Paused:   return L"Paused";
    case ServiceState::Resuming: return L"Resuming";
    case ServiceState::Stopping: return L"Stopping";
    case ServiceState::Error:    return L"Error";
    }
    return L"Unknown";
}

} // namespace DragonOS::Services
