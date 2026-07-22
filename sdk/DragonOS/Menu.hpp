#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace dragonos::sdk {

struct MenuItem {
    uint64_t id{ 0 };
    std::wstring label;
    std::wstring shortcut;
    bool enabled{ true };
    bool checked{ false };
    bool separator{ false };
    std::function<void()> action;
    std::vector<MenuItem> submenu;
};

enum class ContextMenuTarget {
    File,
    Folder,
    Desktop,
    Window,
    Text,
};

struct ContextMenuRequest {
    ContextMenuTarget target{ ContextMenuTarget::Desktop };
    std::wstring targetPath;
    float mouseX{ 0.0f }, mouseY{ 0.0f };
};

class IMenuService {
public:
    virtual ~IMenuService() noexcept = default;

    virtual uint64_t ShowContextMenu(
        const std::vector<MenuItem>& items,
        float x, float y) noexcept = 0;

    virtual void CloseMenu(uint64_t menuId) noexcept = 0;
};

} // namespace dragonos::sdk
