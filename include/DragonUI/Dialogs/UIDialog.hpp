#pragma once

#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/FocusManager.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace DragonOS::DragonUI {

/**
 * @brief  Result code produced when a dialog is closed.
 */
enum class DialogResult : uint8_t {
    None,
    OK,
    Cancel,
    Yes,
    No,
    Retry,
    Abort,
    Ignore,
    Close,
};

/**
 * @brief  A self-contained, draggable, optionally resizable dialog surface.
 *
 * UIDialog renders its own chrome (title bar, border, close button) and
 * manages its own input routing, so it works both under a WindowHost (via
 * DialogManager) and inside legacy render loops (e.g. Explorer) that
 * forward mouse/keyboard events explicitly.
 *
 * Child controls are added with AddContent(); dialog buttons are created
 * with AddButton() and are auto-laid-out along the bottom-right edge.
 */
class UIDialog : public Container {
public:
    using ClosedCallback = std::function<void(UIDialog&, DialogResult)>;

    explicit UIDialog(std::wstring_view title = {}, bool modal = false) noexcept;

    // ── Identity / state ─────────────────────────────────────────────

    void SetTitle(std::wstring_view title) noexcept;
    [[nodiscard]] const std::wstring& GetTitle() const noexcept { return m_title; }

    void SetModal(bool modal) noexcept { m_modal = modal; }
    [[nodiscard]] bool IsModal() const noexcept { return m_modal; }

    [[nodiscard]] bool IsOpen() const noexcept { return m_open; }
    [[nodiscard]] DialogResult GetResult() const noexcept { return m_result; }

    // ── Appearance / behaviour ────────────────────────────────────────

    void SetResizable(bool resizable) noexcept { m_resizable = resizable; }
    [[nodiscard]] bool IsResizable() const noexcept { return m_resizable; }

    void SetCanDrag(bool canDrag) noexcept { m_canDrag = canDrag; }
    [[nodiscard]] bool CanDrag() const noexcept { return m_canDrag; }

    void SetCloseButtonVisible(bool visible) noexcept { m_closeButtonVisible = visible; }

    void SetSize(float width, float height) noexcept;
    void SetMinSize(float width, float height) noexcept;
    void SetPosition(float x, float y) noexcept;
    void CenterIn(float viewportWidth, float viewportHeight) noexcept;
    void SetViewport(float width, float height) noexcept
    {
        m_viewportW = width;
        m_viewportH = height;
    }

    // ── Content ───────────────────────────────────────────────────────

    void AddContent(std::unique_ptr<Element> child) noexcept;
    UIButton* AddButton(std::wstring_view text, DialogResult result) noexcept;
    UIButton* AddButton(std::wstring_view text, UIButton::ClickCallback onClick) noexcept;
    void SetDefaultButton(UIButton* button) noexcept;
    void SetCancelButton(UIButton* button) noexcept;
    [[nodiscard]] UIButton* GetDefaultButton() const noexcept { return m_defaultButton; }

    // ── Lifecycle ─────────────────────────────────────────────────────

    void Show() noexcept;
    void Close(DialogResult result = DialogResult::None) noexcept;

    void SetOnClosed(ClosedCallback cb) noexcept { m_onClosed = std::move(cb); }

    // ── Explicit input routing (works with or without a WindowHost) ──

    bool HandleMouseMove(float x, float y) noexcept;
    bool HandleMouseDown(float x, float y, Input::MouseButton button) noexcept;
    bool HandleMouseUp(float x, float y, Input::MouseButton button) noexcept;
    bool HandleMouseWheel(float delta, float x, float y) noexcept;
    bool HandleKey(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept;
    bool HandleText(wchar_t ch) noexcept;

    [[nodiscard]] bool ContainsPoint(float x, float y) const noexcept
    {
        return m_bounds.Contains(x, y);
    }

    // ── Layout & render ───────────────────────────────────────────────

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Measure(const LayoutSlot& available) noexcept override;
    void Arrange(const LayoutSlot& finalSlot) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    static constexpr float TitleBarHeight = 34.0f;
    static constexpr float BorderThickness = 1.0f;
    static constexpr float ClientPadding = 14.0f;

protected:
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    enum class ResizeEdge : uint8_t {
        None,
        Left, Right, Top, Bottom,
        TopLeft, TopRight, BottomLeft, BottomRight,
    };

    ResizeEdge HitTestResizeEdge(float x, float y) const noexcept;
    bool IsInTitleBarRegion(float x, float y) const noexcept;
    void SetHover(float x, float y) noexcept;
    void DispatchTo(Control* target, const EventArgs& args) noexcept;
    void FocusDefault() noexcept;

    std::wstring m_title;
    bool m_modal{};
    bool m_open{};
    DialogResult m_result{DialogResult::None};
    bool m_resizable{};
    bool m_canDrag{true};
    bool m_closeButtonVisible{true};

    float m_width{480.0f};
    float m_height{320.0f};
    float m_minW{280.0f};
    float m_minH{160.0f};
    float m_viewportW{1280.0f};
    float m_viewportH{720.0f};

    // Internal chrome / layout children (owned via Container::m_children).
    UILabel* m_titleText{};
    UIButton* m_closeButton{};
    UIStackPanel* m_contentPanel{};
    UIStackPanel* m_buttonPanel{};
    UIButton* m_defaultButton{};
    UIButton* m_cancelButton{};

    // Drag / resize state.
    bool m_dragging{};
    float m_dragOffsetX{};
    float m_dragOffsetY{};
    ResizeEdge m_resizeEdge{ResizeEdge::None};
    float m_resizeStartX{};
    float m_resizeStartY{};
    float m_resizeStartW{};
    float m_resizeStartH{};
    float m_resizeStartXpos{};
    float m_resizeStartYpos{};

    // Mini input-pump state (mirrors WindowHost, scoped to this dialog).
    Element* m_pressed{};
    Element* m_hovered{};
    FocusManager m_focusMgr;

    // Double-click detection.
    std::chrono::steady_clock::time_point m_lastClickTime;
    Element* m_lastClickTarget{};
    float m_lastClickX{};
    float m_lastClickY{};

    ClosedCallback m_onClosed;
};

} // namespace DragonOS::DragonUI
