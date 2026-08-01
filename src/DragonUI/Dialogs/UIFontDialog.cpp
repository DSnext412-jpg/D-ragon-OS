#include <DragonUI/Dialogs/UIFontDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace DragonOS::DragonUI {

namespace {

/**
 * @brief  Virtual source over a static list of strings.
 */
class StringSource final : public VirtualItemSource {
public:
    void SetItems(const std::vector<std::wstring>* items) noexcept { m_items = items; }

    [[nodiscard]] int64_t GetCount() const noexcept override
    {
        return m_items ? static_cast<int64_t>(m_items->size()) : 0;
    }

    std::any GetItem(int64_t index) const override
    {
        return (*m_items)[static_cast<size_t>(index)];
    }

private:
    const std::vector<std::wstring>* m_items{};
};

/**
 * @brief  Sample text rendered with a DirectWrite text format created
 *         from the selected family / size / style.
 */
class FontPreview final : public Control {
public:
    FontPreview() noexcept
    {
        SetMinSize(0.0f, 64.0f);
    }

    ~FontPreview() noexcept override
    {
        if (m_fmt) m_fmt->Release();
    }

    void SetFont(std::wstring_view family, float size, bool bold, bool italic) noexcept
    {
        m_family = family;
        m_size = size;
        m_bold = bold;
        m_italic = italic;
        m_dirty = true;
        InvalidateVisual();
    }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override
    {
        return {available.width, 64.0f};
    }

    void Render(RenderContext& ctx) noexcept override
    {
        Element::Render(ctx);

        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        ctx.FillRectangle(d2d, Theme::SemanticColor::ControlFill);

        auto* factory = ctx.Renderer().GetDWriteFactory();
        auto* rt = ctx.Renderer().GetRenderTarget();
        auto* brush = ctx.Renderer().GetBrush(ctx.Resolve(Theme::SemanticColor::TextPrimary));
        if (!factory || !rt || !brush)
            return;

        if (m_dirty || !m_fmt)
        {
            if (m_fmt)
            {
                m_fmt->Release();
                m_fmt = nullptr;
            }
            factory->CreateTextFormat(
                m_family.c_str(),
                nullptr,
                m_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                m_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                (std::clamp)(m_size, 6.0f, 144.0f),
                L"en-us",
                &m_fmt);
            m_dirty = false;
        }

        if (!m_fmt)
            return;

        const std::wstring sample =
            L"The quick brown fox jumps over the lazy dog\n0123456789 ABCabc";
        rt->DrawText(sample.c_str(), static_cast<UINT32>(sample.size()),
            m_fmt, ctx.ScaleRect(d2d), brush);
    }

private:
    std::wstring m_family{L"Segoe UI"};
    float m_size{14.0f};
    bool m_bold{};
    bool m_italic{};
    bool m_dirty{true};
    IDWriteTextFormat* m_fmt{};
};

bool TryParseFloat(std::wstring_view text, float& out)
{
    if (text.empty())
        return false;
    const std::wstring str(text);
    wchar_t* end = nullptr;
    const float value = std::wcstof(str.c_str(), &end);
    if (end == str.c_str() || *end != L'\0')
        return false;
    out = value;
    return true;
}

} // namespace

UIFontDialog::UIFontDialog(
    std::wstring_view title,
    std::wstring_view initialFamily,
    float initialSize,
    bool initialBold,
    bool initialItalic,
    bool modal) noexcept
    : UIDialog(title, modal)
    , m_family(initialFamily)
    , m_size((std::clamp)(initialSize, 6.0f, 144.0f))
    , m_bold(initialBold)
    , m_italic(initialItalic)
{
    m_familyItems = {
        L"Segoe UI", L"Arial", L"Times New Roman", L"Consolas", L"Courier New",
        L"Georgia", L"Verdana", L"Tahoma", L"Trebuchet MS", L"Impact",
        L"Comic Sans MS", L"Lucida Console", L"Franklin Gothic Medium",
    };
    m_familySource = std::make_shared<StringSource>();
    std::static_pointer_cast<StringSource>(m_familySource)->SetItems(&m_familyItems);

    SetResizable(false);
    SetCanDrag(true);
    SetSize(520.0f, 340.0f);
    BuildUi();

    SelectFamily(m_family);
}

void UIFontDialog::BuildUi() noexcept
{
    auto grid = std::make_unique<UIGrid>();
    grid->AddColumn(GridLength::Pixel(210.0f));
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Pixel(72.0f));
    grid->SetColumnSpacing(12.0f);
    grid->SetRowSpacing(10.0f);

    // ── Family list ───────────────────────────────────────────────────
    auto list = std::make_unique<UIListView>();
    list->SetMode(ListViewMode::List);
    list->SetItemHeight(24.0f);
    list->SetHeaderVisible(false);
    list->SetItemSource(m_familySource);
    list->SetPrimaryTextProvider([](const std::any& value) -> std::wstring {
        return std::any_cast<const std::wstring&>(value);
    });
    list->SetOnSelectionChanged([this](const SelectionManager& sm) noexcept {
        const int64_t index = sm.GetFirstSelected();
        if (index < 0)
            return;
        std::any value = m_familyList->GetItemAt(index);
        if (value.has_value())
        {
            try
            {
                m_family = std::any_cast<std::wstring>(value);
                RefreshPreview();
            }
            catch (const std::bad_any_cast&)
            {
            }
        }
    });
    auto listRaw = list.get();
    m_familyList = listRaw;
    grid->AddChild(std::move(list));

    // ── Right column: size + style + preview ─────────────────────────
    auto right = std::make_unique<UIStackPanel>(Orientation::Vertical);
    right->SetSpacing(10.0f);

    auto sizeRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    sizeRow->SetSpacing(8.0f);
    auto sizeLabel = std::make_unique<UILabel>(L"Size");
    auto sizeBox = std::make_unique<UITextBox>(std::to_wstring(static_cast<int>(m_size)));
    sizeBox->SetMaxLength(3);
    sizeBox->SetMaxSize(70.0f, 30.0f);
    sizeBox->SetMinSize(60.0f, 30.0f);
    sizeBox->SetOnTextChanged([this](UITextBox&) noexcept { ApplySizeFromBox(); });
    m_sizeBox = sizeBox.get();
    sizeRow->AddChild(std::move(sizeLabel));
    sizeRow->AddChild(std::move(sizeBox));

    auto boldBtn = std::make_unique<UIButton>(L"Bold");
    boldBtn->SetOnClick([this](UIButton&) noexcept {
        m_bold = !m_bold;
        RefreshPreview();
    });
    m_boldBtn = boldBtn.get();

    auto italicBtn = std::make_unique<UIButton>(L"Italic");
    italicBtn->SetOnClick([this](UIButton&) noexcept {
        m_italic = !m_italic;
        RefreshPreview();
    });
    m_italicBtn = italicBtn.get();

    auto styleRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    styleRow->SetSpacing(8.0f);
    styleRow->AddChild(std::move(boldBtn));
    styleRow->AddChild(std::move(italicBtn));

    right->AddChild(std::move(sizeRow));
    right->AddChild(std::move(styleRow));

    auto rightRaw = right.get();
    grid->AddChild(std::move(right));

    // ── Preview (bottom-right cell) ───────────────────────────────────
    auto preview = std::make_unique<FontPreview>();
    m_preview = preview.get();
    auto previewRaw = preview.get();
    grid->AddChild(std::move(preview));

    grid->SetChildPosition(*listRaw, 0, 0);
    grid->SetChildPosition(*rightRaw, 0, 1);
    grid->SetChildPosition(*previewRaw, 1, 1);

    AddContent(std::move(grid));

    SetDefaultButton(AddButton(L"OK", DialogResult::OK));
    SetCancelButton(AddButton(L"Cancel", DialogResult::Cancel));

    RefreshPreview();
}

void UIFontDialog::SelectFamily(const std::wstring& family) noexcept
{
    if (!m_familyList)
        return;

    int64_t index = 0;
    for (size_t i = 0; i < m_familyItems.size(); ++i)
    {
        if (m_familyItems[i] == family)
        {
            index = static_cast<int64_t>(i);
            break;
        }
    }

    m_familyList->GetSelection().SelectSingle(index);
    m_familyList->GetSelection().SetCurrent(index);
    m_familyList->GetSelection().SetAnchor(index);
    m_familyList->ScrollToItem(index);
}

void UIFontDialog::ApplySizeFromBox() noexcept
{
    if (!m_sizeBox)
        return;
    float value = 0.0f;
    if (TryParseFloat(m_sizeBox->GetText(), value))
        m_size = (std::clamp)(value, 6.0f, 144.0f);
    RefreshPreview();
}

void UIFontDialog::RefreshPreview() noexcept
{
    if (m_boldBtn)
        m_boldBtn->SetText(m_bold ? L"Bold \u2713" : L"Bold");
    if (m_italicBtn)
        m_italicBtn->SetText(m_italic ? L"Italic \u2713" : L"Italic");

    if (m_preview)
    {
        if (auto* pv = dynamic_cast<FontPreview*>(m_preview))
            pv->SetFont(m_family, m_size, m_bold, m_italic);
    }
}

} // namespace DragonOS::DragonUI
