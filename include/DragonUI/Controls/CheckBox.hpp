#pragma once

#include <DragonUI/Core/Control.hpp>
#include <string>
#include <functional>

namespace DragonOS::DragonUI {

enum class CheckState : uint8_t {
    Unchecked,
    Checked,
    Indeterminate,
};

class UICheckBox final : public Control {
public:
    using CheckedChangedCallback = std::function<void(UICheckBox&, CheckState)>;

    explicit UICheckBox(std::wstring_view text = {}) noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetChecked(bool checked) noexcept;
    void SetCheckState(CheckState state) noexcept;
    [[nodiscard]] CheckState GetCheckState() const noexcept { return m_checkState; }
    [[nodiscard]] bool IsChecked() const noexcept { return m_checkState == CheckState::Checked; }
    [[nodiscard]] bool IsIndeterminate() const noexcept { return m_checkState == CheckState::Indeterminate; }

    void SetOnCheckedChanged(CheckedChangedCallback cb) noexcept { m_onCheckedChanged = std::move(cb); }

    void SetAccessibleName(std::wstring_view name) noexcept { m_accessibleName = name; }
    [[nodiscard]] const std::wstring& GetAccessibleName() const noexcept { return m_accessibleName; }
    void SetAccessibleDescription(std::wstring_view desc) noexcept { m_accessibleDescription = desc; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;

private:
    void Toggle() noexcept;

    std::wstring m_text;
    CheckState m_checkState{CheckState::Unchecked};
    CheckedChangedCallback m_onCheckedChanged;

    std::wstring m_accessibleName;
    std::wstring m_accessibleDescription;

    static constexpr float BoxSize = 18.0f;
    static constexpr float Spacing = 6.0f;
};

} // namespace
