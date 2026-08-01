#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>

#include <cstdint>
#include <memory>
#include <string_view>

namespace DragonOS::DragonUI {

/**
 * @brief  Standard message-box icons.
 */
enum class MessageBoxIcon : uint8_t {
    None,
    Info,
    Warning,
    Error,
    Question,
};

/**
 * @brief  Standard message-box button sets.
 */
enum class MessageBoxButtons : uint8_t {
    OK,
    OKCancel,
    YesNo,
    YesNoCancel,
    RetryCancel,
    AbortRetryIgnore,
};

/**
 * @brief  A dialog that shows a message with optional icon and a
 *         standard set of buttons mapped to DialogResult values.
 *
 * @code
 *   auto box = UIMessageBox::Create(
 *       L"Delete", L"Delete this file permanently?",
 *       MessageBoxButtons::YesNo, MessageBoxIcon::Question);
 *   box->SetOnClosed([](UIDialog& dlg, DialogResult r) {
 *       if (r == DialogResult::Yes) { startDelete(); }
 *   });
 *   manager.ShowDialog(std::move(box), host.GetFocusManager());
 * @endcode
 */
class UIMessageBox final : public UIDialog {
public:
    static std::unique_ptr<UIMessageBox> Create(
        std::wstring_view title,
        std::wstring_view message,
        MessageBoxButtons buttons = MessageBoxButtons::OK,
        MessageBoxIcon icon = MessageBoxIcon::None,
        bool modal = true) noexcept;

    [[nodiscard]] const std::wstring& GetMessage() const noexcept { return m_message; }

private:
    UIMessageBox() noexcept = default;

    std::wstring m_message;
};

} // namespace DragonOS::DragonUI
