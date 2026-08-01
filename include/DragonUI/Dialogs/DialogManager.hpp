#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

class FocusManager;

/**
 * @brief  Manages the stack of open dialogs for a host.
 *
 * Handles ownership, modal input blocking, a dimming backdrop for modal
 * dialogs, and restoration of the previously focused control once a modal
 * dialog closes.  Works with WindowHost (see WindowHost::Dialogs()) and can
 * be driven manually by legacy render loops.
 */
class DialogManager final {
public:
    DialogManager() noexcept = default;
    ~DialogManager() noexcept = default;

    DialogManager(const DialogManager&) = delete;
    DialogManager& operator=(const DialogManager&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────

    void ShowDialog(std::unique_ptr<UIDialog> dialog, FocusManager& hostFocus) noexcept;
    void Update(float deltaTime, FocusManager& hostFocus) noexcept;
    void Render(RenderContext& ctx, float viewportWidth, float viewportHeight) noexcept;

    void SetViewport(float width, float height) noexcept
    {
        m_viewportW = width;
        m_viewportH = height;
        for (auto& entry : m_dialogs)
        {
            if (entry.dialog)
                entry.dialog->SetViewport(width, height);
        }
    }

    // ── Input routing (returns true when the event was consumed) ──────

    bool HandleMouseMove(float x, float y) noexcept;
    bool HandleMouseDown(float x, float y, Input::MouseButton button) noexcept;
    bool HandleMouseUp(float x, float y, Input::MouseButton button) noexcept;
    bool HandleMouseWheel(float delta, float x, float y) noexcept;
    bool HandleKey(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept;
    bool HandleText(wchar_t ch) noexcept;

    // ── Query ─────────────────────────────────────────────────────────

    [[nodiscard]] bool IsEmpty() const noexcept { return m_dialogs.empty(); }
    [[nodiscard]] bool HasModalOpen() const noexcept;
    [[nodiscard]] UIDialog* GetTopDialog() const noexcept;
    [[nodiscard]] size_t GetDialogCount() const noexcept { return m_dialogs.size(); }

private:
    struct Entry {
        std::unique_ptr<UIDialog> dialog;
        Control* savedFocus{};
    };

    void SweepClosed(FocusManager& hostFocus) noexcept;

    std::vector<Entry> m_dialogs;
    UIDialog* m_active{};
    float m_viewportW{1280.0f};
    float m_viewportH{720.0f};
};

} // namespace DragonOS::DragonUI
