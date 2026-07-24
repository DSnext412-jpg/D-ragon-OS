#include <UI/Controls/Toolbar.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

Toolbar::Toolbar() noexcept
{
    SetStyle(UIStyle::DefaultToolbar());
    SetAccessibilityRole(L"toolbar");
}

Button* Toolbar::AddButton(std::wstring_view text, wchar_t icon, std::function<void()> callback) noexcept
{
    auto btn = std::make_unique<Button>(text);
    btn->SetIcon(icon);
    btn->SetOnClick(callback);

    auto* raw = btn.get();

    AddChild(std::move(btn));

    InvalidateLayout();
    return raw;
}

D2D1_RECT_F Toolbar::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float totalWidth = static_cast<float>(m_children.size()) * (ButtonWidth + 2.0f);

    float availWidth = available.right - available.left;
    float availHeight = available.bottom - available.top;

    float width = (std::min)((std::max)(totalWidth, 20.0f), availWidth);
    float height = (std::min)(ButtonHeight, availHeight);

    return {0, 0, width, height};
}

void Toolbar::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);

    float currentX = finalRect.left + 4.0f;
    for (auto& child : m_children)
    {
        if (auto* btn = dynamic_cast<Button*>(child.get()))
        {
            D2D1_RECT_F btnRect = {
                currentX, finalRect.top + 2.0f,
                currentX + ButtonWidth - 4.0f, finalRect.bottom - 2.0f
            };
            btn->Arrange(btnRect);
            currentX += ButtonWidth + 2.0f;
        }
    }
}

void Toolbar::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::ExplorerToolbarBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    for (auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Visible)
        {
            if (auto* btn = dynamic_cast<Button*>(child.get()))
            {
                btn->Render(renderer);
            }
        }
    }
}

} // namespace
