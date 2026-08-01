#include <DragonUI/Dialogs/UIColorDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>

namespace DragonOS::DragonUI {

namespace {

std::wstring ByteToHex(uint8_t value)
{
    wchar_t buf[3]{};
    std::swprintf(buf, 3, L"%02X", static_cast<unsigned>(value));
    return buf;
}

int HexCharValue(wchar_t ch)
{
    if (ch >= L'0' && ch <= L'9') return ch - L'0';
    if (ch >= L'a' && ch <= L'f') return ch - L'a' + 10;
    if (ch >= L'A' && ch <= L'F') return ch - L'A' + 10;
    return -1;
}

bool TryParseInt(std::wstring_view text, int& out)
{
    if (text.empty())
        return false;
    long value = 0;
    const std::wstring str(text);
    wchar_t* end = nullptr;
    value = std::wcstol(str.c_str(), &end, 10);
    if (end == str.c_str() || *end != L'\0')
        return false;
    out = static_cast<int>(value);
    return true;
}

/**
 * @brief  Rectangle that renders a user-selected colour.
 */
class ColorPreview final : public Control {
public:
    ColorPreview() noexcept { SetMinSize(72.0f, 72.0f); SetMaxSize(72.0f, 72.0f); }

    void SetColor(Theme::ThemeColor color) noexcept
    {
        m_color = color;
        InvalidateVisual();
    }

    [[nodiscard]] Theme::ThemeColor GetColor() const noexcept { return m_color; }

    DesiredSize MeasureOverride(const LayoutSlot& /*available*/) noexcept override
    {
        return {72.0f, 72.0f};
    }

    void Render(RenderContext& ctx) noexcept override
    {
        Element::Render(ctx);
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        const auto scaled = ctx.ScaleRect(d2d);

        ctx.Renderer().FillRectangle(scaled,
            {m_color.r, m_color.g, m_color.b, m_color.a}, 1.0f);
        ctx.Renderer().DrawRectangle(scaled,
            ctx.Resolve(Theme::SemanticColor::ControlBorder), 1.0f);
    }

private:
    Theme::ThemeColor m_color{};
};

/**
 * @brief  A clickable colour swatch button.
 */
class ColorSwatch final : public Control {
public:
    using ClickCallback = std::function<void(ColorSwatch&)>;

    explicit ColorSwatch(Theme::ThemeColor color) noexcept
        : m_color(color)
    {
        SetMinSize(26.0f, 26.0f);
        SetMaxSize(26.0f, 26.0f);
    }

    void SetColor(Theme::ThemeColor color) noexcept
    {
        m_color = color;
        InvalidateVisual();
    }

    [[nodiscard]] Theme::ThemeColor GetColor() const noexcept { return m_color; }
    void SetOnClick(ClickCallback cb) noexcept { m_onClick = std::move(cb); }

    DesiredSize MeasureOverride(const LayoutSlot& /*available*/) noexcept override
    {
        return {26.0f, 26.0f};
    }

    void Render(RenderContext& ctx) noexcept override
    {
        Element::Render(ctx);
        auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
        const auto scaled = ctx.ScaleRect(d2d);

        ctx.Renderer().FillRectangle(scaled,
            {m_color.r, m_color.g, m_color.b, m_color.a}, 1.0f);
        ctx.Renderer().DrawRectangle(scaled,
            ctx.Resolve(Theme::SemanticColor::ControlBorder), 1.0f);
    }

    bool OnMouseEvent(EventType type, const MouseEventArgs& /*args*/) noexcept override
    {
        if (type == EventType::Click && m_onClick)
        {
            m_onClick(*this);
            return true;
        }
        return false;
    }

private:
    Theme::ThemeColor m_color{};
    ClickCallback m_onClick;
};

std::vector<Theme::ThemeColor> StandardSwatches() noexcept
{
    return {
        Theme::ThemeColor::FromRGB(255, 255, 255),
        Theme::ThemeColor::FromRGB(0, 0, 0),
        Theme::ThemeColor::FromRGB(220, 50, 50),
        Theme::ThemeColor::FromRGB(230, 170, 40),
        Theme::ThemeColor::FromRGB(50, 190, 80),
        Theme::ThemeColor::FromRGB(58, 134, 255),
        Theme::ThemeColor::FromRGB(100, 60, 220),
        Theme::ThemeColor::FromRGB(240, 100, 180),
        Theme::ThemeColor::FromRGB(128, 128, 128),
        Theme::ThemeColor::FromRGB(0, 180, 200),
    };
}

} // namespace

UIColorDialog::UIColorDialog(
    Theme::ThemeColor initial,
    std::wstring_view title,
    bool modal) noexcept
    : UIDialog(title, modal)
    , m_color(initial)
{
    SetResizable(false);
    SetCanDrag(true);
    SetSize(380.0f, 360.0f);
    BuildUi();
}

std::wstring UIColorDialog::GetHexString() const noexcept
{
    const auto r = static_cast<uint8_t>(std::lround(m_color.r * 255.0f));
    const auto g = static_cast<uint8_t>(std::lround(m_color.g * 255.0f));
    const auto b = static_cast<uint8_t>(std::lround(m_color.b * 255.0f));
    return L"#" + ByteToHex(r) + ByteToHex(g) + ByteToHex(b);
}

void UIColorDialog::SetRecentColors(std::vector<Theme::ThemeColor> colors) noexcept
{
    m_recentColors = std::move(colors);
}

void UIColorDialog::BuildUi() noexcept
{
    auto content = std::make_unique<UIStackPanel>(Orientation::Vertical);
    content->SetSpacing(10.0f);

    // ── Preview + fields ──────────────────────────────────────────────
    auto topRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    topRow->SetSpacing(16.0f);

    auto preview = std::make_unique<ColorPreview>();
    preview->SetColor(m_color);
    m_preview = preview.get();

    auto fields = std::make_unique<UIStackPanel>(Orientation::Vertical);
    fields->SetSpacing(6.0f);

    auto MakeField = [&](std::wstring_view label, int maxLen, UITextBox*& out) {
        auto row = std::make_unique<UIStackPanel>(Orientation::Horizontal);
        row->SetSpacing(8.0f);
        auto lbl = std::make_unique<UILabel>(label);
        lbl->SetMinSize(34.0f, 24.0f);
        auto box = std::make_unique<UITextBox>();
        box->SetMaxLength(static_cast<size_t>(maxLen));
        box->SetMaxSize(80.0f, 30.0f);
        box->SetMinSize(64.0f, 30.0f);
        out = box.get();
        row->AddChild(std::move(lbl));
        row->AddChild(std::move(box));
        return row;
    };

    auto hexRow = MakeField(L"Hex", 6, m_hexBox);
    auto rRow = MakeField(L"R", 3, m_rBox);
    auto gRow = MakeField(L"G", 3, m_gBox);
    auto bRow = MakeField(L"B", 3, m_bBox);

    m_hexBox->SetOnTextChanged([this](UITextBox&) noexcept {
        if (!m_updatingFields) UpdateFromHex();
    });
    m_rBox->SetOnTextChanged([this](UITextBox&) noexcept {
        if (!m_updatingFields) UpdateFromRgbBoxes();
    });
    m_gBox->SetOnTextChanged([this](UITextBox&) noexcept {
        if (!m_updatingFields) UpdateFromRgbBoxes();
    });
    m_bBox->SetOnTextChanged([this](UITextBox&) noexcept {
        if (!m_updatingFields) UpdateFromRgbBoxes();
    });

    fields->AddChild(std::move(hexRow));
    fields->AddChild(std::move(rRow));
    fields->AddChild(std::move(gRow));
    fields->AddChild(std::move(bRow));

    auto previewRaw = preview.get();
    topRow->AddChild(std::move(preview));
    topRow->AddChild(std::move(fields));

    auto topRaw = topRow.get();
    content->AddChild(std::move(topRow));

    // ── Theme swatches ────────────────────────────────────────────────
    auto themeLabel = std::make_unique<UILabel>(L"Theme colours");
    themeLabel->SetTextColor(Theme::SemanticColor::TextSecondary);
    content->AddChild(std::move(themeLabel));

    auto themeRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    themeRow->SetSpacing(6.0f);
    for (auto color : StandardSwatches())
    {
        auto sw = std::make_unique<ColorSwatch>(color);
        sw->SetOnClick([this](ColorSwatch& s) noexcept { SetColor(s.GetColor()); });
        themeRow->AddChild(std::move(sw));
    }
    auto themeRowRaw = themeRow.get();
    content->AddChild(std::move(themeRow));

    // ── Recent swatches ───────────────────────────────────────────────
    auto recentLabel = std::make_unique<UILabel>(L"Recent colours");
    recentLabel->SetTextColor(Theme::SemanticColor::TextSecondary);
    content->AddChild(std::move(recentLabel));

    auto recentRow = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    recentRow->SetSpacing(6.0f);
    for (auto color : m_recentColors)
    {
        auto sw = std::make_unique<ColorSwatch>(color);
        sw->SetOnClick([this](ColorSwatch& s) noexcept { SetColor(s.GetColor()); });
        recentRow->AddChild(std::move(sw));
    }
    auto recentRowRaw = recentRow.get();
    content->AddChild(std::move(recentRow));

    AddContent(std::move(content));

    SetDefaultButton(AddButton(L"OK", DialogResult::OK));
    SetCancelButton(AddButton(L"Cancel", DialogResult::Cancel));

    (void)topRaw;
    (void)themeRowRaw;
    (void)recentRowRaw;
    (void)previewRaw;

    RefreshFields();
}

void UIColorDialog::SetColor(Theme::ThemeColor color) noexcept
{
    m_color = color;
    RefreshFields();
}

void UIColorDialog::RefreshFields() noexcept
{
    m_updatingFields = true;

    const auto r = static_cast<int>(std::lround(m_color.r * 255.0f));
    const auto g = static_cast<int>(std::lround(m_color.g * 255.0f));
    const auto b = static_cast<int>(std::lround(m_color.b * 255.0f));

    if (m_hexBox) m_hexBox->SetText(ByteToHex(static_cast<uint8_t>(r)) +
                                    ByteToHex(static_cast<uint8_t>(g)) +
                                    ByteToHex(static_cast<uint8_t>(b)));
    if (m_rBox) m_rBox->SetText(std::to_wstring(r));
    if (m_gBox) m_gBox->SetText(std::to_wstring(g));
    if (m_bBox) m_bBox->SetText(std::to_wstring(b));

    if (m_preview)
    {
        if (auto* pv = dynamic_cast<ColorPreview*>(m_preview))
            pv->SetColor(m_color);
    }

    m_updatingFields = false;
}

void UIColorDialog::UpdateFromRgbBoxes() noexcept
{
    int r = 0, g = 0, b = 0;
    const bool okR = m_rBox && TryParseInt(m_rBox->GetText(), r);
    const bool okG = m_gBox && TryParseInt(m_gBox->GetText(), g);
    const bool okB = m_bBox && TryParseInt(m_bBox->GetText(), b);
    if (!okR || !okG || !okB)
        return;

    r = (std::clamp)(r, 0, 255);
    g = (std::clamp)(g, 0, 255);
    b = (std::clamp)(b, 0, 255);

    const Theme::ThemeColor next = Theme::ThemeColor::FromRGB(
        static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255);

    m_color = next;
    m_updatingFields = true;
    if (m_hexBox)
        m_hexBox->SetText(ByteToHex(static_cast<uint8_t>(r)) +
                          ByteToHex(static_cast<uint8_t>(g)) +
                          ByteToHex(static_cast<uint8_t>(b)));
    if (m_preview)
    {
        if (auto* pv = dynamic_cast<ColorPreview*>(m_preview))
            pv->SetColor(next);
    }
    m_updatingFields = false;
}

void UIColorDialog::UpdateFromHex() noexcept
{
    if (!m_hexBox)
        return;

    int values[6];
    int count = 0;
    for (wchar_t ch : m_hexBox->GetText())
    {
        if (ch == L'#')
            continue;
        if (count >= 6)
            return;
        const int v = HexCharValue(ch);
        if (v < 0)
            return;
        values[count++] = v;
    }
    if (count != 6)
        return;

    const int r = values[0] * 16 + values[1];
    const int g = values[2] * 16 + values[3];
    const int b = values[4] * 16 + values[5];

    const Theme::ThemeColor next = Theme::ThemeColor::FromRGB(
        static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255);

    m_color = next;
    m_updatingFields = true;
    if (m_rBox) m_rBox->SetText(std::to_wstring(r));
    if (m_gBox) m_gBox->SetText(std::to_wstring(g));
    if (m_bBox) m_bBox->SetText(std::to_wstring(b));
    if (m_preview)
    {
        if (auto* pv = dynamic_cast<ColorPreview*>(m_preview))
            pv->SetColor(next);
    }
    m_updatingFields = false;
}

} // namespace DragonOS::DragonUI
