#pragma once

#include <DragonUI/Core/Control.hpp>
#include <string>
#include <functional>
#include <vector>

namespace DragonOS::DragonUI {

class UIRadioButton;

class RadioGroup final {
public:
    void AddButton(UIRadioButton* button) noexcept;
    void RemoveButton(UIRadioButton* button) noexcept;
    void Select(UIRadioButton* button) noexcept;
    [[nodiscard]] UIRadioButton* GetSelected() const noexcept { return m_selected; }
    void ClearSelection() noexcept;

private:
    std::vector<UIRadioButton*> m_buttons;
    UIRadioButton* m_selected{};
};

class UIRadioButton final : public Control {
public:
    using CheckedCallback = std::function<void(UIRadioButton&)>;

    explicit UIRadioButton(std::wstring_view text = {}) noexcept;
    ~UIRadioButton() noexcept;

    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetChecked(bool checked) noexcept;
    [[nodiscard]] bool IsChecked() const noexcept { return m_checked; }

    void SetGroup(RadioGroup* group) noexcept;
    [[nodiscard]] RadioGroup* GetGroup() const noexcept { return m_group; }

    void SetOnCheckedChanged(CheckedCallback cb) noexcept { m_onCheckedChanged = std::move(cb); }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;

private:
    void NotifyGroup() noexcept;

    std::wstring m_text;
    bool m_checked{};
    RadioGroup* m_group{};
    CheckedCallback m_onCheckedChanged;

    static constexpr float DotSize = 18.0f;
    static constexpr float InnerDotSize = 8.0f;
    static constexpr float Spacing = 6.0f;
};

} // namespace
