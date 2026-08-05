#pragma once

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/FocusManager.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <DragonUI/Accessibility/AccessibilityManager.hpp>
#include <DragonUI/Accessibility/AccessibilityInspector.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>

#include <memory>

namespace DragonOS::DragonUI {

class DialogManager;

class WindowHost final {
public:
    WindowHost(Graphics::Renderer& renderer, Theme::ThemeManager& theme, Input::InputManager& input) noexcept;
    ~WindowHost() noexcept;

    WindowHost(const WindowHost&) = delete;
    WindowHost& operator=(const WindowHost&) = delete;
    WindowHost(WindowHost&&) = delete;
    WindowHost& operator=(WindowHost&&) = delete;

    void SetRoot(std::unique_ptr<Element> root) noexcept;
    [[nodiscard]] Element* GetRoot() const noexcept { return m_root.get(); }

    void SetDpiScale(float scale) noexcept;
    [[nodiscard]] float GetDpiScale() const noexcept { return m_dpiScale; }

    void Update(float deltaTime) noexcept;
    void Render() noexcept;
    void Resize(float width, float height) noexcept;

    [[nodiscard]] FocusManager& GetFocusManager() noexcept { return m_focusMgr; }

    /// @brief  Accessibility / UI Automation service for this host.
    [[nodiscard]] AccessibilityManager& GetAccessibility() noexcept { return m_accessibility; }
    [[nodiscard]] const AccessibilityManager& GetAccessibility() const noexcept { return m_accessibility; }

    /// @brief  Open / close the Accessibility Inspector developer tool.
    void ToggleInspector() noexcept;
    [[nodiscard]] bool IsInspectorOpen() const noexcept { return m_inspector != nullptr; }

    [[nodiscard]] DialogManager& Dialogs() const noexcept { return *m_dialogs; }

    void OnMouseMove(float x, float y) noexcept;
    void OnMouseDown(float x, float y, Input::MouseButton button) noexcept;
    void OnMouseUp(float x, float y, Input::MouseButton button) noexcept;
    void OnMouseLeave() noexcept;
    void OnMouseWheel(float delta, float x, float y) noexcept;
    void OnKeyDown(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept;
    void OnTextInput(wchar_t ch) noexcept;

private:
    Control* HitTestControl(float x, float y) noexcept;
    void DispatchEvent(Control* target, const EventArgs& args) noexcept;
    void UpdateHover(float x, float y) noexcept;
    void OnFocusChanged(Control* focused) noexcept;
    void PlaceInspector() noexcept;

    Graphics::Renderer& m_renderer;
    Theme::ThemeManager& m_theme;
    Input::InputManager& m_input;
    std::unique_ptr<Element> m_root;
    std::unique_ptr<DialogManager> m_dialogs;
    FocusManager m_focusMgr;
    AccessibilityManager m_accessibility;
    std::unique_ptr<AccessibilityInspector> m_inspector;
    float m_dpiScale{1.0f};
    float m_viewportW{};
    float m_viewportH{};
    Control* m_hovered{};
    Control* m_pressed{};
    bool m_mouseInBounds{};
};

} // namespace
