#pragma once

#include <DragonUI/Core/Container.hpp>
#include <functional>
#include <string>

namespace DragonOS::DragonUI {

class UIStatusBar final : public Container {
public:
    UIStatusBar() noexcept = default;

    void SetStatusText(std::wstring_view text) noexcept;
    [[nodiscard]] const std::wstring& GetStatusText() const noexcept;

    void SetIcon(uint32_t glyph) noexcept;
    [[nodiscard]] uint32_t GetIcon() const noexcept { return m_iconGlyph; }

    void SetProgress(float value) noexcept;
    [[nodiscard]] float GetProgress() const noexcept { return m_progress; }
    [[nodiscard]] bool HasProgress() const noexcept { return m_progress >= 0.0f; }
    void ClearProgress() noexcept;

    void SetLeftPanelMinWidth(float width) noexcept { m_leftMinWidth = width; InvalidateLayout(); }
    void SetRightPanelMinWidth(float width) noexcept { m_rightMinWidth = width; InvalidateLayout(); }

    void SetLeftPanelContent(std::unique_ptr<Element> content) noexcept;
    void SetRightPanelContent(std::unique_ptr<Element> content) noexcept;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

    static constexpr float DefaultHeight = 24.0f;
    static constexpr float PaddingH = 8.0f;
    static constexpr float PaddingV = 2.0f;
    static constexpr float IconSize = 14.0f;
    static constexpr float ProgressWidth = 100.0f;
    static constexpr float ProgressHeight = 6.0f;
    static constexpr float MinPanelWidth = 60.0f;

private:
    std::wstring m_statusText;
    uint32_t m_iconGlyph{};
    float m_progress{ -1.0f };
    float m_leftMinWidth{ MinPanelWidth };
    float m_rightMinWidth{ MinPanelWidth };

    std::unique_ptr<Element> m_leftPanel;
    std::unique_ptr<Element> m_rightPanel;
};

} // namespace
