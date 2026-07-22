#pragma once

#include <cstdint>
#include <string>

namespace DragonOS::Command {

enum class CommandStatus : uint8_t {
    Success,
    Failure,
    InvalidArgument,
    NotFound,
    PermissionDenied,
    InternalError
};

struct CommandResult final {
    CommandStatus status{ CommandStatus::Success };
    std::wstring  output;
    std::wstring  error;
    uint64_t      executionTimeUs{ 0 };

    [[nodiscard]] bool IsSuccess() const noexcept { return status == CommandStatus::Success; }
    [[nodiscard]] bool IsFailure() const noexcept { return status != CommandStatus::Success; }
};

} // namespace DragonOS::Command
