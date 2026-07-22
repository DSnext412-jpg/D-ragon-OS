#pragma once

#include <DragonOS/Menu.hpp>

#include <cstdint>
#include <map>
#include <vector>

namespace DragonOS::SDK {

class MenuServiceAdapter final : public dragonos::sdk::IMenuService {
public:
    MenuServiceAdapter() noexcept = default;

    uint64_t ShowContextMenu(
        const std::vector<dragonos::sdk::MenuItem>& items,
        float x, float y) noexcept override;
    void CloseMenu(uint64_t menuId) noexcept override;

private:
    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    struct ActiveMenu {
        uint64_t id;
        std::vector<dragonos::sdk::MenuItem> items;
        float x, y;
    };

    std::vector<ActiveMenu> m_activeMenus;
};

} // namespace DragonOS::SDK
