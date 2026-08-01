#pragma once

#include <cstdint>
#include <any>

namespace DragonOS::DragonUI {

/**
 * @brief  Abstract source of data items for virtualized controls.
 *
 * Controls only ever materialize the items that intersect the visible
 * viewport.  A source backed by tens of thousands of records never
 * allocates more than a handful of item objects per frame.
 *
 * Implementors must be able to return a value for any index in
 * [0, GetCount()).  The returned std::any is passed to item templates
 * for rendering.
 */
class VirtualItemSource {
public:
    virtual ~VirtualItemSource() = default;

    [[nodiscard]] virtual int64_t GetCount() const noexcept = 0;
    [[nodiscard]] virtual std::any GetItem(int64_t index) const = 0;
};

} // namespace DragonOS::DragonUI
