#pragma once

#include <DragonUI/Core/Container.hpp>

namespace DragonOS::DragonUI {

class Panel : public Container {
public:
    Panel() noexcept = default;

    Alignment horizontalAlignment{Alignment::Stretch};
    Alignment verticalAlignment{Alignment::Stretch};

    void SetHorizontalAlignment(Alignment a) noexcept { horizontalAlignment = a; InvalidateLayout(); }
    void SetVerticalAlignment(Alignment a) noexcept { verticalAlignment = a; InvalidateLayout(); }

    [[nodiscard]] Alignment GetHorizontalAlignment() const noexcept { return horizontalAlignment; }
    [[nodiscard]] Alignment GetVerticalAlignment() const noexcept { return verticalAlignment; }
};

} // namespace
