#pragma once

#include <DragonUI/Core/Panel.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/PasswordBox.hpp>
#include <DragonUI/Controls/CheckBox.hpp>
#include <DragonUI/Controls/RadioButton.hpp>
#include <DragonUI/Controls/ToggleSwitch.hpp>
#include <memory>

namespace DragonOS::DragonUI::Demo {

class DemoPanel : public Panel {
public:
    explicit DemoPanel(const Thickness& windowMargin) noexcept;
    void Arrange(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
protected:
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;
private:
    Thickness m_windowMargin;
};

class DemoText : public Element {
public:
    explicit DemoText(std::wstring_view text) noexcept;
    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }
    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
private:
    std::wstring m_text;
};

class DemoButton : public Control {
public:
    explicit DemoButton(std::wstring_view text) noexcept;
    void SetText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetText() const noexcept { return m_text; }
    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
private:
    std::wstring m_text;
};

// ── Input Controls Demo ─────────────────────────────────────────────

struct InputControlsDemo {
    static std::unique_ptr<Element> Create() noexcept;
};

// ── Layout Containers Demo ─────────────────────────────────────────

struct LayoutDemo {
    static std::unique_ptr<Element> Create() noexcept;
};

} // namespace
