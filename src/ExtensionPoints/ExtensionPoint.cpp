#include <ExtensionPoints/ExtensionPoint.hpp>

#include <algorithm>

namespace DragonOS::ExtensionPoints {

void ExtensionPointManager::Register(
    ExtensionPointType type,
    std::unique_ptr<IExtensionPoint> point) noexcept
{
    if (!point) { return; }
    ExtensionEntry entry{ std::move(point), type };
    m_points.push_back(std::move(entry));
}

void ExtensionPointManager::Unregister(std::wstring_view name) noexcept
{
    m_points.erase(
        std::remove_if(m_points.begin(), m_points.end(),
            [&](const auto& entry) { return entry.point->GetName() == name; }),
        m_points.end());
}

void ExtensionPointManager::UnregisterPlugin(std::wstring_view pluginName) noexcept
{
    m_points.erase(
        std::remove_if(m_points.begin(), m_points.end(),
            [&](const auto& entry) { return entry.point->GetPluginName() == pluginName; }),
        m_points.end());
}

std::vector<IExtensionPoint*> ExtensionPointManager::GetByType(
    ExtensionPointType type) const noexcept
{
    std::vector<IExtensionPoint*> result;
    for (const auto& entry : m_points)
    {
        if (entry.type == type)
        {
            result.push_back(entry.point.get());
        }
    }
    return result;
}

IExtensionPoint* ExtensionPointManager::Find(std::wstring_view name) const noexcept
{
    for (const auto& entry : m_points)
    {
        if (entry.point->GetName() == name)
        {
            return entry.point.get();
        }
    }
    return nullptr;
}

size_t ExtensionPointManager::GetCountByType(ExtensionPointType type) const noexcept
{
    size_t count = 0;
    for (const auto& entry : m_points)
    {
        if (entry.type == type) { ++count; }
    }
    return count;
}

void ExtensionPointManager::Clear() noexcept
{
    m_points.clear();
}

} // namespace DragonOS::ExtensionPoints
