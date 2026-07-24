#pragma once

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>
#include <Theme/ThemeTypography.hpp>
#include <d2d1.h>
#include <stack>

namespace DragonOS::DragonUI {

class RenderContext final {
public:
    RenderContext(Graphics::Renderer& renderer, const Theme::ThemeManager& theme, float dpiScale = 1.0f) noexcept
        : m_renderer(&renderer), m_theme(&theme), m_dpiScale(dpiScale) {}

    [[nodiscard]] Graphics::Renderer& Renderer() const noexcept { return *m_renderer; }
    [[nodiscard]] const Theme::ThemeManager& Theme() const noexcept { return *m_theme; }
    [[nodiscard]] float DpiScale() const noexcept { return m_dpiScale; }

    [[nodiscard]] Graphics::Color Resolve(Theme::SemanticColor token) const noexcept {
        const auto& tc = m_theme->GetColor(token);
        return {tc.r, tc.g, tc.b, tc.a};
    }

    [[nodiscard]] D2D1_RECT_F ScaleRect(const D2D1_RECT_F& rect) const noexcept {
        return {
            rect.left * m_dpiScale,
            rect.top * m_dpiScale,
            rect.right * m_dpiScale,
            rect.bottom * m_dpiScale
        };
    }

    void PushClip(const D2D1_RECT_F& rect) noexcept {
        auto scaled = ScaleRect(rect);
        m_renderer->GetRenderTarget()->PushAxisAlignedClip(scaled, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_clipStack.push(scaled);
    }

    void PopClip() noexcept {
        m_renderer->GetRenderTarget()->PopAxisAlignedClip();
        if (!m_clipStack.empty()) m_clipStack.pop();
    }

    void FillRectangle(const D2D1_RECT_F& rect, Theme::SemanticColor token, float opacity = 1.0f) noexcept {
        m_renderer->FillRectangle(ScaleRect(rect), Resolve(token), opacity);
    }

    void DrawRectangle(const D2D1_RECT_F& rect, Theme::SemanticColor token, float strokeWidth = 1.0f) noexcept {
        m_renderer->DrawRectangle(ScaleRect(rect), Resolve(token), strokeWidth * m_dpiScale);
    }

    void DrawText(std::wstring_view text, const D2D1_RECT_F& layoutRect, Theme::SemanticColor token) noexcept {
        m_renderer->DrawText(text, ScaleRect(layoutRect), Resolve(token));
    }

private:
    Graphics::Renderer* m_renderer;
    const Theme::ThemeManager* m_theme;
    float m_dpiScale{1.0f};
    std::stack<D2D1_RECT_F> m_clipStack;
};

} // namespace
