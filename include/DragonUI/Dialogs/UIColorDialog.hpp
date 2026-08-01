#pragma once

#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Controls/TextBox.hpp>
#include <DragonUI/Controls/Label.hpp>
#include <DragonUI/Controls/StackPanel.hpp>
#include <DragonUI/Controls/Grid.hpp>

#include <Theme/ThemeColor.hpp>

#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  Colour picker dialog.
 *
 * Provides RGB and HEX fields, a live preview swatch, a set of theme
 * swatches and a row of recently used colours.  The picked colour is
 * read via GetSelectedColor() once the dialog closes with OK.
 */
class UIColorDialog final : public UIDialog {
public:
    explicit UIColorDialog(
        Theme::ThemeColor initial = Theme::ThemeColor{},
        std::wstring_view title = L"Select Color",
        bool modal = true) noexcept;

    [[nodiscard]] Theme::ThemeColor GetSelectedColor() const noexcept { return m_color; }
    [[nodiscard]] std::wstring GetHexString() const noexcept;

    void SetRecentColors(std::vector<Theme::ThemeColor> colors) noexcept;

private:
    void BuildUi() noexcept;
    void SetColor(Theme::ThemeColor color) noexcept;
    void UpdateFromRgbBoxes() noexcept;
    void UpdateFromHex() noexcept;
    void RefreshFields() noexcept;

    Theme::ThemeColor m_color;
    std::vector<Theme::ThemeColor> m_recentColors;

    UITextBox* m_hexBox{};
    UITextBox* m_rBox{};
    UITextBox* m_gBox{};
    UITextBox* m_bBox{};
    Control* m_preview{};
    bool m_updatingFields{};
};

} // namespace DragonOS::DragonUI
