#pragma once

#include <cstdint>

namespace DragonOS::Input {

enum class CursorType : uint8_t {
    Arrow,
    Hand,
    ResizeHorizontal,
    ResizeVertical,
    ResizeDiagonalNWSE,
    ResizeDiagonalNESW,
    Text,
    Wait,
    Cross,
    Move,
};

class CursorManager final {
public:
    CursorManager() noexcept;
    ~CursorManager() noexcept = default;

    CursorManager(const CursorManager&)            = delete;
    CursorManager& operator=(const CursorManager&) = delete;

    void SetCursor(CursorType type) noexcept;
    [[nodiscard]] CursorType GetCurrentCursor() const noexcept { return m_current; }

    void Apply() noexcept;

private:
    CursorType m_current{ CursorType::Arrow };
    bool       m_dirty{ true };
};

}
