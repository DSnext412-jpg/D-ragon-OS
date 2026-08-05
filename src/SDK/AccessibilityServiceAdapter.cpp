#include "AccessibilityServiceAdapter.hpp"

namespace DragonOS::SDK {

bool AccessibilityServiceAdapter::IsHighContrast() const noexcept
{
    return m_manager.Preferences().IsHighContrast();
}

void AccessibilityServiceAdapter::SetHighContrast(bool enabled) noexcept
{
    m_manager.Preferences().SetHighContrast(enabled);
    m_manager.ApplyPreferences();
}

bool AccessibilityServiceAdapter::IsReducedMotion() const noexcept
{
    return m_manager.Preferences().IsReducedMotion();
}

void AccessibilityServiceAdapter::SetReducedMotion(bool enabled) noexcept
{
    m_manager.Preferences().SetReducedMotion(enabled);
}

bool AccessibilityServiceAdapter::IsLargeFonts() const noexcept
{
    return m_manager.Preferences().IsLargeFonts();
}

void AccessibilityServiceAdapter::SetTextScale(float scale) noexcept
{
    m_manager.Preferences().SetTextScale(scale);
}

float AccessibilityServiceAdapter::GetTextScale() const noexcept
{
    return m_manager.Preferences().GetTextScale();
}

bool AccessibilityServiceAdapter::IsScreenReaderActive() const noexcept
{
    return m_manager.Preferences().IsScreenReaderActive();
}

void AccessibilityServiceAdapter::Announce(std::wstring_view text) noexcept
{
    m_manager.Announce(text);
}

void AccessibilityServiceAdapter::Alert(std::wstring_view text) noexcept
{
    m_manager.Alert(text);
}

void AccessibilityServiceAdapter::ReportStructuredChange() noexcept
{
    m_manager.ReportStructuredChange();
}

uint64_t AccessibilityServiceAdapter::GetNodeCount() const noexcept
{
    return m_manager.Tree().GetNodeCount();
}

uint64_t AccessibilityServiceAdapter::GetTreeDepth() const noexcept
{
    return m_manager.Tree().GetDepth();
}

} // namespace DragonOS::SDK
