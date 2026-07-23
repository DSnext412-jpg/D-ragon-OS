#pragma once
#include <vector>
#include <cstdint>

namespace DragonOS::UI {

class UIElement;
class UIContainer;

class FocusManager final {
public:
    void SetFocusedElement(UIElement* element) noexcept;
    [[nodiscard]] UIElement* GetFocusedElement() const noexcept { return m_focused; }

    void FocusNext() noexcept;
    void FocusPrevious() noexcept;
    void FocusFirst() noexcept;
    void FocusLast() noexcept;

    void RegisterRoot(UIContainer* root) noexcept;
    void UnregisterRoot(UIContainer* root) noexcept;
    void UpdateTabOrder() noexcept;

    [[nodiscard]] bool HasFocus() const noexcept { return m_focused != nullptr; }
    [[nodiscard]] bool IsFocused(const UIElement* element) const noexcept { return m_focused == element; }

    void SetFocusIndicatorVisible(bool visible) noexcept { m_showFocusIndicator = visible; }
    [[nodiscard]] bool GetFocusIndicatorVisible() const noexcept { return m_showFocusIndicator; }

private:
    void CollectFocusable(UIElement* element, std::vector<UIElement*>& out) noexcept;

    UIElement* m_focused{nullptr};
    std::vector<UIContainer*> m_roots;
    std::vector<UIElement*> m_tabOrder;
    bool m_showFocusIndicator{true};
};

} // namespace
