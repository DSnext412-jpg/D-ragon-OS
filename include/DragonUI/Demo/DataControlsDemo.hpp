#pragma once

#include <DragonUI/Core/Element.hpp>
#include <memory>

namespace DragonOS::DragonUI {

class WindowHost;

namespace Demo {

/**
 * @brief  Data controls demo shown by the DragonUISystem shell.
 *
 * Contains:
 *   - UIGridView  — Plugin Manager demo (name, version, author, enabled, status)
 *   - UIListView  — virtualized file list (50,000 rows) with icons
 *   - UITreeView  — nested folder navigation with lazy loading
 *   - Dialog framework — MessageBox / Open / Save / Folder / Color / Font / Progress
 */
struct DataControlsDemo {
    static std::unique_ptr<Element> Create(WindowHost& host) noexcept;
};

/**
 * @brief  Standalone Plugin Manager demo (a UIGridView of plugins).
 */
struct PluginManagerDemo {
    static std::unique_ptr<Element> Create() noexcept;
};

} // namespace DragonOS::DragonUI::Demo

} // namespace DragonOS::DragonUI
