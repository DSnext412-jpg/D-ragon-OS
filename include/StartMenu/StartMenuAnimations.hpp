#pragma once

#include <Animation/Animation.hpp>
#include <Animation/AnimationCurve.hpp>

namespace DragonOS::StartMenu {

class FloatAnimation final : public ::DragonOS::Animation::Animation {
public:
    FloatAnimation(
        float* target,
        float  startValue,
        float  endValue,
        float  duration,
        ::DragonOS::Animation::AnimationCurve curve = ::DragonOS::Animation::AnimationCurve::EaseOut) noexcept
        : Animation(duration, curve)
        , m_pTarget(target)
        , m_startValue(startValue)
        , m_endValue(endValue)
    {
    }

protected:
    void OnUpdate(float progress) noexcept override
    {
        if (m_pTarget)
        {
            *m_pTarget = m_startValue + (m_endValue - m_startValue) * progress;
        }
    }

private:
    float* m_pTarget;
    float  m_startValue;
    float  m_endValue;
};

} // namespace DragonOS::StartMenu
