#include <Apps/ApplicationRegistry.hpp>

namespace DragonOS::Apps {

// ============================================================================
//  Lifecycle
// ============================================================================

bool ApplicationRegistry::InitializeInternal() noexcept
{
    if (m_initialized) { return true; }

    // ── Pre-populate with demo applications ───────────────────────────────

    auto reg = [&](std::wstring name, std::wstring display,
                   AppCategory cat, bool pinned)
    {
        AppInfo info;
        info.id          = NextId();
        info.name        = std::move(name);
        info.displayName = std::move(display);
        info.category    = cat;
        info.pinned      = pinned;
        Register(std::move(info));
    };

    reg(L"FileExplorer", L"File Explorer",      AppCategory::System,        true);
    reg(L"Terminal",     L"Terminal",            AppCategory::System,        true);
    reg(L"Settings",     L"Settings",            AppCategory::System,        true);
    reg(L"Notepad",      L"Notepad",             AppCategory::Utilities,     true);
    reg(L"Calculator",   L"Calculator",          AppCategory::Utilities,     true);
    reg(L"Browser",      L"Web Browser",         AppCategory::Internet,      true);
    reg(L"Mail",         L"Mail",                AppCategory::Productivity,  true);
    reg(L"Calendar",     L"Calendar",            AppCategory::Productivity,  false);
    reg(L"Maps",         L"Maps",                AppCategory::Utilities,     false);
    reg(L"Photos",       L"Photos",              AppCategory::Media,         false);
    reg(L"Music",        L"Music Player",        AppCategory::Media,         false);
    reg(L"VSCode",       L"Visual Studio Code",  AppCategory::Development,   false);
    reg(L"GitGUI",       L"Git GUI",             AppCategory::Development,   false);
    reg(L"Paint",        L"Paint",               AppCategory::Graphics,      false);
    reg(L"Solitaire",    L"Solitaire",           AppCategory::Games,         false);

    m_initialized = true;
    return true;
}

void ApplicationRegistry::ShutdownInternal() noexcept
{
    if (!m_initialized) { return; }
    m_apps.clear();
    m_initialized = false;
}

// ============================================================================
//  Registration
// ============================================================================

uint64_t ApplicationRegistry::Register(AppInfo info) noexcept
{
    if (info.id == 0)
    {
        info.id = NextId();
    }

    const uint64_t id = info.id;
    m_apps[id] = std::move(info);
    return id;
}

bool ApplicationRegistry::Unregister(uint64_t appId) noexcept
{
    return m_apps.erase(appId) > 0;
}

// ============================================================================
//  Queries
// ============================================================================

AppInfo* ApplicationRegistry::Find(uint64_t appId) noexcept
{
    auto it = m_apps.find(appId);
    return (it != m_apps.end()) ? &it->second : nullptr;
}

const AppInfo* ApplicationRegistry::Find(uint64_t appId) const noexcept
{
    auto it = m_apps.find(appId);
    return (it != m_apps.end()) ? &it->second : nullptr;
}

AppInfo* ApplicationRegistry::FindByName(std::wstring_view name) noexcept
{
    for (auto& [id, info] : m_apps)
    {
        if (info.name == name)
            return &info;
    }
    return nullptr;
}

const AppInfo* ApplicationRegistry::FindByName(std::wstring_view name) const noexcept
{
    for (const auto& [id, info] : m_apps)
    {
        if (info.name == name)
            return &info;
    }
    return nullptr;
}

std::vector<const AppInfo*> ApplicationRegistry::GetPinned() const noexcept
{
    std::vector<const AppInfo*> result;
    result.reserve(m_apps.size());

    for (const auto& [id, info] : m_apps)
    {
        if (info.pinned && !info.hidden)
        {
            result.push_back(&info);
        }
    }

    return result;
}

std::vector<const AppInfo*> ApplicationRegistry::GetAll() const noexcept
{
    std::vector<const AppInfo*> result;
    result.reserve(m_apps.size());

    for (const auto& [id, info] : m_apps)
    {
        if (!info.hidden)
        {
            result.push_back(&info);
        }
    }

    return result;
}

std::vector<const AppInfo*> ApplicationRegistry::GetByCategory(AppCategory cat) const noexcept
{
    std::vector<const AppInfo*> result;
    result.reserve(m_apps.size());

    for (const auto& [id, info] : m_apps)
    {
        if (info.category == cat && !info.hidden)
        {
            result.push_back(&info);
        }
    }

    return result;
}

void ApplicationRegistry::SetPinned(uint64_t appId, bool pinned) noexcept
{
    auto* info = Find(appId);
    if (info)
    {
        info->pinned = pinned;
    }
}

} // namespace DragonOS::Apps
