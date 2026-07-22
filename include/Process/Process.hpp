#pragma once

#include <Process/ProcessHandle.hpp>

#include <cstdint>
#include <string>

namespace DragonOS::Process {

enum class ProcessState : uint8_t {
    Running,
    Background,
    Suspended,
    Terminated
};

class Process final {
public:
    Process(
        uint64_t       processId,
        std::wstring   name,
        bool           isBackground = false) noexcept
        : m_handle{ processId }
        , m_name{ std::move(name) }
        , m_state{ isBackground ? ProcessState::Background : ProcessState::Running }
    {
    }

    [[nodiscard]] const ProcessHandle& GetHandle() const noexcept { return m_handle; }
    [[nodiscard]] uint64_t GetProcessId() const noexcept { return m_handle.GetProcessId(); }
    [[nodiscard]] const std::wstring& GetName() const noexcept { return m_name; }
    [[nodiscard]] ProcessState GetState() const noexcept { return m_state; }
    [[nodiscard]] bool IsBackground() const noexcept { return m_state == ProcessState::Background; }

    void SetState(ProcessState state) noexcept { m_state = state; }
    void Suspend() noexcept { m_state = ProcessState::Suspended; }
    void Resume() noexcept { m_state = ProcessState::Running; }
    void Terminate() noexcept { m_state = ProcessState::Terminated; }

private:
    ProcessHandle m_handle;
    std::wstring  m_name;
    ProcessState  m_state{ ProcessState::Running };
};

} // namespace DragonOS::Process
