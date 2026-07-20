#pragma once

#include <Animation/Animation.hpp>

#include <memory>

namespace DragonOS::Animation {
enum class AnimationCurve;
}

namespace DragonOS::WindowManager {

class DragonWindow;

std::unique_ptr<::DragonOS::Animation::Animation> CreateMoveAnimation(
    DragonWindow* window,
    float targetX, float targetY,
    float duration = 0.15f,
    ::DragonOS::Animation::AnimationCurve curve = ::DragonOS::Animation::AnimationCurve::EaseInOut) noexcept;

std::unique_ptr<::DragonOS::Animation::Animation> CreateResizeAnimation(
    DragonWindow* window,
    float targetW, float targetH,
    float duration = 0.15f,
    ::DragonOS::Animation::AnimationCurve curve = ::DragonOS::Animation::AnimationCurve::EaseInOut) noexcept;

std::unique_ptr<::DragonOS::Animation::Animation> CreateTransformAnimation(
    DragonWindow* window,
    float targetX, float targetY,
    float targetW, float targetH,
    float duration = 0.15f,
    ::DragonOS::Animation::AnimationCurve curve = ::DragonOS::Animation::AnimationCurve::EaseInOut) noexcept;

} // namespace DragonOS::WindowManager
