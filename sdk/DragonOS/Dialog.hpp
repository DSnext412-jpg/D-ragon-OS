#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace dragonos::sdk {

enum class DialogResult {
    None,
    OK,
    Cancel,
    Yes,
    No,
    Retry,
};

struct DialogParams {
    std::wstring title;
    std::wstring message;
    std::vector<std::wstring> buttons{ L"OK" };
    std::wstring icon; // "info", "warning", "error", "question"
};

class IDialogService {
public:
    virtual ~IDialogService() noexcept = default;

    virtual DialogResult ShowMessageBox(const DialogParams& params) noexcept = 0;

    using DialogCallback = std::function<void(DialogResult)>;
    virtual void ShowMessageBoxAsync(
        const DialogParams& params,
        DialogCallback callback) noexcept = 0;

    virtual std::wstring OpenFileDialog(
        std::wstring_view title,
        std::wstring_view filter) noexcept = 0;

    virtual std::wstring SaveFileDialog(
        std::wstring_view title,
        std::wstring_view filter,
        std::wstring_view defaultName) noexcept = 0;

    virtual std::wstring OpenFolderDialog(
        std::wstring_view title) noexcept = 0;
};

} // namespace dragonos::sdk
