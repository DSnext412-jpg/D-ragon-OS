#pragma once

#include <DragonUI/Accessibility/UIAAutomationTree.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

class Element;
class UIPanel;
class UIStackPanel;
class UILabel;
class UIButton;
class AccessibilityManager;

/**
 * @brief  Developer tool that visualises the accessibility tree.
 *
 * Displays, for every node in the tree:
 *   - role
 *   - accessible name
 *   - automation id
 *   - state flags
 *   - value / bounds
 *
 * The header also shows the current keyboard focus and exposes toggles for
 * high-contrast, reduced motion and large fonts so the accessibility
 * preferences can be validated live.  Refresh is manual (F5 / button) so
 * the tool never pays the tree-build cost on its own.
 */
class AccessibilityInspector final {
public:
    explicit AccessibilityInspector(AccessibilityManager& manager) noexcept;
    ~AccessibilityInspector() noexcept;

    AccessibilityInspector(const AccessibilityInspector&) = delete;
    AccessibilityInspector& operator=(const AccessibilityInspector&) = delete;

    /// @brief  Build (or rebuild) the inspector view.
    /// @return The top-level view element (ownership transfers to the caller).
    std::unique_ptr<Element> CreateView() noexcept;

    /// @brief  Re-read the automation tree and update every label.
    void Refresh() noexcept;

    /// @brief  Whether the inspector view has been created.
    [[nodiscard]] bool HasView() const noexcept { return m_view != nullptr; }

    /// @brief  The current top-level view (may be null).
    [[nodiscard]] Element* GetView() const noexcept { return m_view; }

    /// @brief  Report the element that has keyboard focus.
    void SetFocusedElement(Element* element) noexcept { m_focusedElement = element; }

private:
    void BuildHeader(UIStackPanel& header) noexcept;
    void RebuildTreeRows() noexcept;
    void AppendNodeRow(AccessibleElement* node, size_t depth) noexcept;
    std::wstring FormatState(AccessibilityState state) const noexcept;
    std::wstring FormatBounds(const LayoutSlot& bounds) const noexcept;

    AccessibilityManager& m_manager;
    Element* m_view{};
    UIStackPanel* m_treePanel{};
    UILabel* m_focusLabel{};
    UILabel* m_statusLabel{};
    UIButton* m_hcButton{};
    UIButton* m_motionButton{};
    UIButton* m_fontsButton{};
    Element* m_focusedElement{};
};

} // namespace DragonOS::DragonUI