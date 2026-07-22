#include "DialogServiceAdapter.hpp"

#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>

#include <optional>
#include <string>

namespace DragonOS::SDK {

dragonos::sdk::DialogResult DialogServiceAdapter::ShowMessageBox(
    const dragonos::sdk::DialogParams& params) noexcept
{
    std::wstring buttons;
    if (params.buttons.size() == 1 && params.buttons[0] == L"OK")
    {
        buttons = L"OK";
    }

    UINT type = MB_OK;
    if (params.buttons.size() >= 2)
    {
        if (params.buttons[0] == L"OK" && params.buttons[1] == L"Cancel")
            type = MB_OKCANCEL;
        else if (params.buttons[0] == L"Yes" && params.buttons[1] == L"No")
            type = params.buttons.size() > 2 ? MB_YESNOCANCEL : MB_YESNO;
        else if (params.buttons[0] == L"Retry" && params.buttons[1] == L"Cancel")
            type = MB_RETRYCANCEL;
    }

    if (params.icon == L"error")       type |= MB_ICONERROR;
    else if (params.icon == L"warning") type |= MB_ICONWARNING;
    else if (params.icon == L"question")type |= MB_ICONQUESTION;
    else if (params.icon == L"info")    type |= MB_ICONINFORMATION;

    int result = ::MessageBoxW(
        nullptr,
        params.message.c_str(),
        params.title.c_str(),
        type);

    switch (result)
    {
    case IDOK:     return dragonos::sdk::DialogResult::OK;
    case IDCANCEL: return dragonos::sdk::DialogResult::Cancel;
    case IDYES:    return dragonos::sdk::DialogResult::Yes;
    case IDNO:     return dragonos::sdk::DialogResult::No;
    case IDRETRY:  return dragonos::sdk::DialogResult::Retry;
    default:       return dragonos::sdk::DialogResult::Cancel;
    }
}

void DialogServiceAdapter::ShowMessageBoxAsync(
    const dragonos::sdk::DialogParams& params,
    dragonos::sdk::IDialogService::DialogCallback callback) noexcept
{
    auto result = ShowMessageBox(params);
    if (callback)
    {
        callback(result);
    }
}

std::wstring DialogServiceAdapter::OpenFileDialog(
    std::wstring_view title,
    std::wstring_view filter) noexcept
{
    wchar_t buffer[4096] = { 0 };

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = std::size(buffer);
    ofn.lpstrTitle = title.data() ? std::wstring{ title }.c_str() : nullptr;
    ofn.lpstrFilter = std::wstring{ filter }.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (::GetOpenFileNameW(&ofn))
    {
        return std::wstring{ buffer };
    }

    return {};
}

std::wstring DialogServiceAdapter::SaveFileDialog(
    std::wstring_view title,
    std::wstring_view filter,
    std::wstring_view defaultName) noexcept
{
    wchar_t buffer[4096] = { 0 };

    if (!defaultName.empty())
    {
        ::wcsncpy_s(buffer, std::wstring{ defaultName }.c_str(), _TRUNCATE);
    }

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = std::size(buffer);
    ofn.lpstrTitle = title.data() ? std::wstring{ title }.c_str() : nullptr;
    ofn.lpstrFilter = std::wstring{ filter }.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (::GetSaveFileNameW(&ofn))
    {
        return std::wstring{ buffer };
    }

    return {};
}

std::wstring DialogServiceAdapter::OpenFolderDialog(
    std::wstring_view title) noexcept
{
    wchar_t buffer[4096] = { 0 };

    BROWSEINFOW bi = {};
    bi.hwndOwner = nullptr;
    bi.lpszTitle = title.data() ? std::wstring{ title }.c_str() : nullptr;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = ::SHBrowseForFolderW(&bi);
    if (pidl)
    {
        if (::SHGetPathFromIDListW(pidl, buffer))
        {
            IMalloc* pMalloc = nullptr;
            if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
            return std::wstring{ buffer };
        }
        IMalloc* pMalloc = nullptr;
        if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
        {
            pMalloc->Free(pidl);
            pMalloc->Release();
        }
    }

    return {};
}

} // namespace DragonOS::SDK
