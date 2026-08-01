#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/Grid.hpp>
#include <DragonUI/Core/VirtualItemSource.hpp>

#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  Font selection dialog (family, weight, style, size) with a
 *         live preview rendered through DirectWrite.
 */
class UIFontDialog final : public UIDialog {
public:
    UIFontDialog(
        std::wstring_view title = L"Select Font",
        std::wstring_view initialFamily = L"Segoe UI",
        float initialSize = 14.0f,
        bool initialBold = false,
        bool initialItalic = false,
        bool modal = true) noexcept;

    [[nodiscard]] const std::wstring& GetSelectedFamily() const noexcept { return m_family; }
    [[nodiscard]] float GetSelectedSize() const noexcept { return m_size; }
    [[nodiscard]] bool GetSelectedBold() const noexcept { return m_bold; }
    [[nodiscard]] bool GetSelectedItalic() const noexcept { return m_italic; }

private:
    void BuildUi() noexcept;
    void RefreshPreview() noexcept;
    void SelectFamily(const std::wstring& family) noexcept;
    void ApplySizeFromBox() noexcept;

    std::wstring m_family;
    float m_size{14.0f};
    bool m_bold{};
    bool m_italic{};

    std::vector<std::wstring> m_familyItems;
    std::shared_ptr<VirtualItemSource> m_familySource;

    UIListView* m_familyList{};
    UITextBox* m_sizeBox{};
    UIButton* m_boldBtn{};
    UIButton* m_italicBtn{};
    Control* m_preview{};
};

} // namespace DragonOS::DragonUI
