#pragma once

#include <functional>
#include <vector>
#include <cstdint>

namespace DragonOS::DragonUI {

class Element;
class Container;
class Control;

enum class FocusDirection : uint8_t {
    Up,
    Down,
    Left,
    Right,
};

class FocusManager final {
public:
    using FocusChangedCallback = std::function<void(Control*)>;

    void SetFocus(Control* element) noexcept;
    [[nodiscard]] Control* GetFocused() const noexcept { return m_focused; }

    /// @brief  Observes every focus change (used by accessibility wiring).
    void SetOnFocusChanged(FocusChangedCallback cb) noexcept { m_onFocusChanged = std::move(cb); }

    void FocusNext() noexcept;
    void FocusPrevious() noexcept;
    void FocusFirst() noexcept;
    void FocusLast() noexcept;

    /// @brief  Logical arrow-key navigation using element bounds.
    void MoveFocusDirection(FocusDirection direction) noexcept;

    /// @brief  Finds the first focusable control whose access key matches.
    [[nodiscard]] Control* FindByAccessKey(wchar_t key, const Control* skip = nullptr) const noexcept;

    /// @brief  Invokes the control currently holding focus (Enter/Space activation).
    void ActivateFocused() noexcept;

    void RegisterRoot(Container* root) noexcept;
    void UnregisterRoot(Container* root) noexcept;
    void RebuildTabOrder() noexcept;

    [[nodiscard]] bool HasFocus() const noexcept { return m_focused != nullptr; }
    [[nodiscard]] bool IsFocused(const Element* element) const noexcept;

    void SetFocusVisible(bool visible) noexcept { m_showFocus = visible; }
    [[nodiscard]] bool GetFocusVisible() const noexcept { return m_showFocus; }

private:
    void CollectFocusable(Element* element, std::vector<Control*>& out) noexcept;

    Control* m_focused{};
    std::vector<Container*> m_roots;
    std::vector<Control*> m_tabOrder;
    bool m_showFocus{true};
    FocusChangedCallback m_onFocusChanged;
};

} // namespace
