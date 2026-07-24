#pragma once

#include <vector>
#include <cstdint>

namespace DragonOS::DragonUI {

class Element;
class Container;
class Control;

class FocusManager final {
public:
    void SetFocus(Control* element) noexcept;
    [[nodiscard]] Control* GetFocused() const noexcept { return m_focused; }

    void FocusNext() noexcept;
    void FocusPrevious() noexcept;
    void FocusFirst() noexcept;
    void FocusLast() noexcept;

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
};

} // namespace
