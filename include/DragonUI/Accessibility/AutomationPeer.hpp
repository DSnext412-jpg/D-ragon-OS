#pragma once

#include <DragonUI/Accessibility/AccessibleElement.hpp>

#include <memory>
#include <string>

namespace DragonOS::DragonUI {

// Forward declarations of the concrete controls.
class UIButton;
class UILabel;
class UITextBox;
class UIPasswordBox;
class UICheckBox;
class UIRadioButton;
class UIToggleSwitch;
class UIMenu;
class UIMenuBar;
class UIContextMenu;
class UIMenuItem;
class UIProgressBar;
class UIListView;
class UITreeView;
class UIGridView;
class UIDialog;
class UIImage;
class UISeparator;
class UIStatusBar;
class UIToolBar;

/**
 * @brief  Control patterns exposed by an automation peer.
 *
 * Mirrors the UIA control patterns a screen reader / automation client
 * would query.  A peer advertises the patterns it supports; the pattern's
 * data is read through the peer's state/value accessors.
 */
enum class AutomationPattern : uint32_t {
    None            = 0x00,
    Invoke          = 0x01, ///< Button-like: can be invoked (clicked).
    Value           = 0x02, ///< Holds an editable value (text box).
    Selection       = 0x04, ///< Contains a selectable set (list / tree / grid).
    SelectionItem   = 0x08, ///< Is a selectable member of a selection set.
    ExpandCollapse  = 0x10, ///< Can be expanded / collapsed (tree nodes, menus).
    Toggle          = 0x20, ///< Has a toggleable on/off state (check, switch).
    RangeValue      = 0x40, ///< Has a numeric range (progress bar, slider).
    Text            = 0x80, ///< Exposes textual content (label, read-only).
};

inline constexpr AutomationPattern operator|(AutomationPattern a, AutomationPattern b) noexcept
{
    return static_cast<AutomationPattern>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

/**
 * @brief  Base class for control-specific automation peers.
 *
 * Every DragonUI control has a peer produced by AutomationPeer::CreateFor.
 * A peer advertises the automation patterns it supports, resolves the
 * control's accessibility role and computes state / value from the live
 * control.
 */
class AutomationPeer : public AccessibleElement {
public:
    explicit AutomationPeer(Element* element) noexcept;

    [[nodiscard]] AutomationPattern GetPatterns() const noexcept { return m_patterns; }

protected:
    void SetPatterns(AutomationPattern patterns) noexcept { m_patterns = patterns; }

    /// @brief  OR additional state flags into the node (peer subclasses).
    void AddState(AccessibilityState state) noexcept { m_stateOverride |= state; }

private:
    AutomationPattern m_patterns{ AutomationPattern::None };
};

// ── Per-control peers ─────────────────────────────────────────────────────

class ButtonPeer final : public AutomationPeer {
public:
    explicit ButtonPeer(UIButton* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    void Invoke() noexcept;   ///< Programmatic activation.
};

class LabelPeer final : public AutomationPeer {
public:
    explicit LabelPeer(UILabel* element) noexcept;
    std::wstring_view ComputeValue() const noexcept override;
};

class TextBoxPeer final : public AutomationPeer {
public:
    explicit TextBoxPeer(UITextBox* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    std::wstring_view ComputeValue() const noexcept override;
};

class PasswordBoxPeer final : public AutomationPeer {
public:
    explicit PasswordBoxPeer(UIPasswordBox* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    std::wstring_view ComputeValue() const noexcept override;
};

class CheckBoxPeer final : public AutomationPeer {
public:
    explicit CheckBoxPeer(UICheckBox* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    std::wstring_view ComputeValue() const noexcept override;
};

class RadioButtonPeer final : public AutomationPeer {
public:
    explicit RadioButtonPeer(UIRadioButton* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
};

class ToggleSwitchPeer final : public AutomationPeer {
public:
    explicit ToggleSwitchPeer(UIToggleSwitch* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    std::wstring_view ComputeValue() const noexcept override;
};

class MenuPeer final : public AutomationPeer {
public:
    explicit MenuPeer(Element* element) noexcept;
    AccessibilityRole ResolveRole() const noexcept override;
};

class MenuItemPeer final : public AutomationPeer {
public:
    explicit MenuItemPeer(UIMenuItem* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    std::wstring_view ComputeValue() const noexcept override;
};

class ProgressBarPeer final : public AutomationPeer {
public:
    explicit ProgressBarPeer(UIProgressBar* element) noexcept;
    std::wstring_view ComputeValue() const noexcept override;

private:
    mutable std::wstring m_valueBuffer;
};

class ListViewPeer final : public AutomationPeer {
public:
    explicit ListViewPeer(UIListView* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    bool BuildVirtualChildren() noexcept override;
};

class TreeViewPeer final : public AutomationPeer {
public:
    explicit TreeViewPeer(UITreeView* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    bool BuildVirtualChildren() noexcept override;
};

class GridViewPeer final : public AutomationPeer {
public:
    explicit GridViewPeer(UIGridView* element) noexcept;
    AccessibilityState ComputeState() const noexcept override;
    bool BuildVirtualChildren() noexcept override;
};

class DialogPeer final : public AutomationPeer {
public:
    explicit DialogPeer(UIDialog* element) noexcept;
    AccessibilityRole ResolveRole() const noexcept override;
    AccessibilityState ComputeState() const noexcept override;
};

class ImagePeer final : public AutomationPeer {
public:
    explicit ImagePeer(UIImage* element) noexcept;
};

class SeparatorPeer final : public AutomationPeer {
public:
    explicit SeparatorPeer(UISeparator* element) noexcept;
};

class StatusBarPeer final : public AutomationPeer {
public:
    explicit StatusBarPeer(UIStatusBar* element) noexcept;
};

class ToolBarPeer final : public AutomationPeer {
public:
    explicit ToolBarPeer(UIToolBar* element) noexcept;
};

/**
 * @brief  Factory producing the correct automation peer for an element.
 *
 * @return A peer for @p element, or nullptr when @p element is null.
 */
[[nodiscard]] std::unique_ptr<AccessibleElement> AutomationPeerFactory(Element* element) noexcept;

/// @brief  Best-effort role inference used as a fallback for custom elements.
[[nodiscard]] AccessibilityRole InferAccessibleRole(Element* element) noexcept;

} // namespace DragonOS::DragonUI