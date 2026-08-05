#pragma once

#include <DragonOS/Accessibility.hpp>

#include <DragonUI/Accessibility/AccessibilityManager.hpp>

namespace DragonOS::SDK {

class AccessibilityServiceAdapter final : public dragonos::sdk::IAccessibilityService {
public:
    explicit AccessibilityServiceAdapter(
        DragonUI::AccessibilityManager& mgr) noexcept
        : m_manager{ mgr }
    {
    }

    bool IsHighContrast() const noexcept override;
    void SetHighContrast(bool enabled) noexcept override;

    bool IsReducedMotion() const noexcept override;
    void SetReducedMotion(bool enabled) noexcept override;

    bool IsLargeFonts() const noexcept override;
    void SetTextScale(float scale) noexcept override;
    float GetTextScale() const noexcept override;

    bool IsScreenReaderActive() const noexcept override;

    void Announce(std::wstring_view text) noexcept override;
    void Alert(std::wstring_view text) noexcept override;
    void ReportStructuredChange() noexcept override;

    uint64_t GetNodeCount() const noexcept override;
    uint64_t GetTreeDepth() const noexcept override;

private:
    DragonUI::AccessibilityManager& m_manager;
};

} // namespace DragonOS::SDK
