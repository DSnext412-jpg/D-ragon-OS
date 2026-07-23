#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <UI/Core/UIElement.hpp>

namespace DragonOS::UI::Animation {

enum class Easing : uint8_t {
    Linear,
    EaseIn, EaseOut, EaseInOut,
    BounceOut,
};

struct AnimatedProperty {
    enum Type : uint8_t {
        Opacity, OffsetX, OffsetY, Scale
    };
    Type type;
    float from, to;
    float duration;
    Easing easing{Easing::EaseOut};
};

class UIAnimation final {
public:
    UIAnimation(UIElement* target, AnimatedProperty prop) noexcept;

    void Start() noexcept;
    void Stop() noexcept;
    void Pause() noexcept;
    void Resume() noexcept;
    [[nodiscard]] bool IsRunning() const noexcept { return m_running; }
    [[nodiscard]] bool IsCompleted() const noexcept { return m_completed; }

    void Update(float deltaTime) noexcept;
    void SetOnComplete(std::function<void()> callback) noexcept { m_onComplete = std::move(callback); }

private:
    float ApplyEasing(float t) const noexcept;

    UIElement* m_target;
    AnimatedProperty m_property;
    float m_elapsed{0};
    bool m_running{false};
    bool m_completed{false};
    bool m_paused{false};
    std::function<void()> m_onComplete;
};

class TransitionManager final {
public:
    [[nodiscard]] UIAnimation* Animate(UIElement* target, AnimatedProperty prop) noexcept;
    void Update(float deltaTime) noexcept;
    void Clear() noexcept;
    void StopAll() noexcept;

    void FadeIn(UIElement* target, float duration = 0.3f) noexcept;
    void FadeOut(UIElement* target, float duration = 0.3f) noexcept;
    void SlideIn(UIElement* target, float fromX, float fromY, float duration = 0.3f) noexcept;
    void ScaleIn(UIElement* target, float duration = 0.3f) noexcept;

    void AnimateStateTransition(UIElement* target, ElementState oldState, ElementState newState) noexcept;

private:
    std::vector<std::unique_ptr<UIAnimation>> m_animations;
};

} // namespace
