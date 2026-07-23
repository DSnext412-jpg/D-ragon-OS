#pragma once
#include <cstdint>

namespace DragonOS::UI {

enum class Orientation : uint8_t { Horizontal, Vertical };
enum class Alignment : uint8_t { Start, Center, End, Stretch };
enum class DockPosition : uint8_t { Left, Top, Right, Bottom, Fill };

struct GridDefinition {
    float size{0};
    bool isStar{false};
    bool isAuto{true};
};

} // namespace
