#pragma once

#include <DragonOS/Dialog.hpp>

#include <string>

namespace DragonOS::SDK {

class DialogServiceAdapter final : public dragonos::sdk::IDialogService {
public:
    DialogServiceAdapter() noexcept = default;

    dragonos::sdk::DialogResult ShowMessageBox(
        const dragonos::sdk::DialogParams& params) noexcept override;

    void ShowMessageBoxAsync(
        const dragonos::sdk::DialogParams& params,
        dragonos::sdk::IDialogService::DialogCallback callback) noexcept override;

    std::wstring OpenFileDialog(
        std::wstring_view title,
        std::wstring_view filter) noexcept override;

    std::wstring SaveFileDialog(
        std::wstring_view title,
        std::wstring_view filter,
        std::wstring_view defaultName) noexcept override;

    std::wstring OpenFolderDialog(
        std::wstring_view title) noexcept override;
};

} // namespace DragonOS::SDK
