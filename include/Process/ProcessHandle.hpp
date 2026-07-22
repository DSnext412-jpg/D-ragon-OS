#pragma once

#include <cstdint>

namespace DragonOS::Process {

class ProcessHandle final {
public:
    ProcessHandle() noexcept = default;

    explicit ProcessHandle(uint64_t processId) noexcept
        : m_processId{ processId }
    {
    }

    [[nodiscard]] uint64_t GetProcessId() const noexcept { return m_processId; }
    [[nodiscard]] bool IsValid() const noexcept { return m_processId != 0; }

    void Reset() noexcept { m_processId = 0; }

private:
    uint64_t m_processId{ 0 };
};

} // namespace DragonOS::Process
