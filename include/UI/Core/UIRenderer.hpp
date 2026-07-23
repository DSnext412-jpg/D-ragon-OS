#pragma once
#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include "UIElement.hpp"

namespace DragonOS::UI {

class UIRenderer final {
public:
    UIRenderer(Graphics::Renderer& renderer, const Theme::ThemeManager& theme) noexcept
        : m_renderer(&renderer), m_theme(&theme) {}

    void SetRenderer(Graphics::Renderer& renderer) noexcept { m_renderer = &renderer; }
    void SetTheme(const Theme::ThemeManager& theme) noexcept { m_theme = &theme; }

    [[nodiscard]] Graphics::Renderer& GetRenderer() noexcept { return *m_renderer; }
    [[nodiscard]] const Theme::ThemeManager& GetTheme() const noexcept { return *m_theme; }

    [[nodiscard]] Graphics::Color ResolveColor(Theme::SemanticColor token) const noexcept;

    void FillBackground(const D2D1_RECT_F& rect, const StyleStateColors& colors, float cornerRadius, float opacity = 1.0f) noexcept;
    void DrawBorder(const D2D1_RECT_F& rect, const StyleStateColors& colors, float thickness, float cornerRadius, float opacity = 1.0f) noexcept;
    void DrawText(std::wstring_view text, const D2D1_RECT_F& layoutRect, const StyleStateColors& colors) noexcept;
    void DrawIcon(const D2D1_RECT_F& rect, wchar_t glyph, const StyleStateColors& colors, float fontSize = 16.0f) noexcept;
    void FillGauge(const D2D1_RECT_F& rect, float progress, const StyleStateColors& colors, float cornerRadius = 4.0f) noexcept;
    void DrawFocusIndicator(const D2D1_RECT_F& rect) noexcept;

    void PushClip(const D2D1_RECT_F& clipRect) noexcept;
    void PopClip() noexcept;

private:
    Graphics::Renderer* m_renderer;
    const Theme::ThemeManager* m_theme;
};

} // namespace
