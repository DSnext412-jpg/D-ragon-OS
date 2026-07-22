#include "MenuServiceAdapter.hpp"

#include <algorithm>

namespace DragonOS::SDK {

uint64_t MenuServiceAdapter::ShowContextMenu(
    const std::vector<dragonos::sdk::MenuItem>& items,
    float x, float y) noexcept
{
    if (items.empty()) { return 0; }

    ActiveMenu menu;
    menu.id = NextId();
    menu.items = items;
    menu.x = x;
    menu.y = y;
    m_activeMenus.push_back(std::move(menu));

    return menu.id;
}

void MenuServiceAdapter::CloseMenu(uint64_t menuId) noexcept
{
    m_activeMenus.erase(
        std::remove_if(m_activeMenus.begin(), m_activeMenus.end(),
            [menuId](const auto& m) { return m.id == menuId; }),
        m_activeMenus.end());
}

} // namespace DragonOS::SDK
