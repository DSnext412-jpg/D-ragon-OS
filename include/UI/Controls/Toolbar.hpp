#pragma once
#include <UI/Core/UIContainer.hpp>
#include <UI/Controls/Button.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

class Toolbar : public UIContainer {
public:
    Toolbar() noexcept;

    Button* AddButton(std::wstring_view text, wchar_t icon = 0, std::function<void()> callback = nullptr) noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;

    static constexpr float ButtonWidth = 36.0f;
    static constexpr float ButtonHeight = 28.0f;
};

} // namespace
