#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>

#include <memory>
#include <string>

namespace DragonOS::DragonUI {

/**
 * @brief  Modeless progress dialog with an optional cancel button.
 *
 * The caller drives it from its own update loop:
 *
 * @code
 *   auto dlg = std::make_unique<UIProgressDialog>(L"Copying...", true, true);
 *   dialogManager.ShowDialog(std::move(dlg), host.GetFocusManager());
 *   // each frame:  dialog->SetProgress(pct); dialog->SetStatus(...);
 *   //              if (dialog->IsCancelled()) { break; }
 * @endcode
 */
class UIProgressDialog final : public UIDialog {
public:
    UIProgressDialog(
        std::wstring_view title = L"Progress",
        bool modal = false,
        bool cancellable = true) noexcept;

    void SetProgress(float value) noexcept;
    [[nodiscard]] float GetProgress() const noexcept;

    void SetStatus(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetStatus() const noexcept { return m_status; }

    void SetIndeterminate(bool indeterminate) noexcept;
    [[nodiscard]] bool IsCancelled() const noexcept { return m_cancelled; }

private:
    UIProgressBar* m_progress{};
    UILabel* m_statusLabel{};
    std::wstring m_status;
    bool m_cancelled{};
};

} // namespace DragonOS::DragonUI
