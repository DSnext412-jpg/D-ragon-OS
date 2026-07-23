#pragma once
#include <UI/Core/UIElement.hpp>
#include <string>
#include <functional>
#include <vector>

namespace DragonOS::UI {

class RadioButtonGroup;

class RadioButton : public UIElement {
public:
    RadioButton() noexcept;
    explicit RadioButton(std::wstring_view text) noexcept;

    void SetText(std::wstring_view text) noexcept { m_text = text; InvalidateVisual(); }
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }

    void SetChecked(bool checked) noexcept;
    [[nodiscard]] bool IsChecked() const noexcept { return m_checked; }

    void SetGroup(RadioButtonGroup* group) noexcept { m_group = group; }
    [[nodiscard]] RadioButtonGroup* GetGroup() const noexcept { return m_group; }

    void SetOnCheckedChanged(std::function<void(bool)> cb) noexcept { m_onCheckedChanged = std::move(cb); }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

private:
    std::wstring m_text;
    bool m_checked{false};
    RadioButtonGroup* m_group{nullptr};
    std::function<void(bool)> m_onCheckedChanged;
    static constexpr float CircleSize = 18.0f;
};

class RadioButtonGroup final {
public:
    void Add(RadioButton* rb) noexcept { m_buttons.push_back(rb); rb->SetGroup(this); }
    void Remove(RadioButton* rb) noexcept;
    void Select(RadioButton* rb) noexcept;
    [[nodiscard]] RadioButton* GetSelected() const noexcept { return m_selected; }

private:
    std::vector<RadioButton*> m_buttons;
    RadioButton* m_selected{nullptr};
};

} // namespace
