#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <Theme/ThemePalette.hpp>

namespace DragonOS::UI {

enum class ElementState : uint8_t;

struct StyleStateColors {
    Theme::SemanticColor background{Theme::SemanticColor::WindowBackground};
    Theme::SemanticColor foreground{Theme::SemanticColor::TextPrimary};
    Theme::SemanticColor border{Theme::SemanticColor::WindowBorder};
};

struct StyleAnimationConfig {
    bool enableHover{true};
    bool enablePress{true};
    bool enableFocus{true};
    float hoverDuration{0.2f};
    float pressDuration{0.1f};
    float focusDuration{0.2f};
    float fadeDuration{0.3f};
};

class UIStyle {
public:
    explicit UIStyle(std::string_view name) noexcept : m_name(name) {}

    [[nodiscard]] std::string_view GetName() const noexcept { return m_name; }

    StyleStateColors normal;
    StyleStateColors hover;
    StyleStateColors pressed;
    StyleStateColors focused;
    StyleStateColors disabled;

    std::wstring fontFamily{L"Segoe UI"};
    float fontSize{14.0f};
    bool fontBold{false};

    float cornerRadius{4.0f};
    float borderThickness{1.0f};
    float paddingLeft{8}, paddingTop{4}, paddingRight{8}, paddingBottom{4};
    float marginLeft{0}, marginTop{0}, marginRight{0}, marginBottom{0};

    StyleAnimationConfig animation;

    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultButton() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultLabel() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultTextBox() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultCheckBox() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultToggleSwitch() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultSlider() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultProgressBar() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultComboBox() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultListView() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultScrollViewer() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultTabControl() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultMenu() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultToolbar() noexcept;
    [[nodiscard]] static std::shared_ptr<UIStyle> DefaultStatusBar() noexcept;

    [[nodiscard]] const StyleStateColors& ResolveState(ElementState state) const noexcept;

private:
    std::string m_name;
};

} // namespace
