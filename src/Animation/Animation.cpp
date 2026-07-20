/**
 * @file    Animation.cpp
 * @brief   Implementation of the Animation base class.
 */

#include <Animation/Animation.hpp>

namespace DragonOS::Animation {

Animation::Animation(float duration, AnimationCurve curve) noexcept
    : m_duration{ duration }
    , m_curve{ curve }
{
}

void Animation::Play() noexcept
{
    switch (m_state)
    {
    case AnimationState::Idle:
    case AnimationState::Finished:
        m_elapsed = 0.0f;
        m_state   = AnimationState::Playing;
        break;

    case AnimationState::Paused:
        m_state = AnimationState::Playing;
        break;

    case AnimationState::Playing:
        break; // already playing
    }
}

void Animation::Pause() noexcept
{
    if (m_state == AnimationState::Playing)
    {
        m_state = AnimationState::Paused;
    }
}

void Animation::Stop() noexcept
{
    m_elapsed = 0.0f;
    m_state   = AnimationState::Idle;
}

void Animation::Reset() noexcept
{
    m_elapsed = 0.0f;
    if (m_state == AnimationState::Finished)
    {
        m_state = AnimationState::Idle;
    }
}

void Animation::Update(float deltaTime) noexcept
{
    if (m_state != AnimationState::Playing) { return; }

    if (m_duration <= 0.0f)
    {
        // Zero-duration animation — instantly finish.
        if (m_loop)
        {
            OnUpdate(1.0f);
        }
        else
        {
            OnUpdate(1.0f);
            OnFinished();
            m_state = AnimationState::Finished;
        }
        return;
    }

    m_elapsed += deltaTime;

    // Handle looping.
    if (m_loop && m_elapsed >= m_duration)
    {
        m_elapsed = 0.0f;
        OnUpdate(GetEasedProgress());
        return;
    }

    // Clamp past the end.
    if (m_elapsed >= m_duration)
    {
        m_elapsed = m_duration;
        OnUpdate(GetEasedProgress());
        OnFinished();
        m_state = AnimationState::Finished;
        return;
    }

    // Normal frame update.
    OnUpdate(GetEasedProgress());
}

float Animation::GetProgress() const noexcept
{
    if (m_duration <= 0.0f) { return 1.0f; }

    float t = m_elapsed / m_duration;
    if (m_reverse) { t = 1.0f - t; }
    return (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
}

float Animation::GetEasedProgress() const noexcept
{
    return Easing::Apply(m_curve, GetProgress());
}

void Animation::OnFinished() noexcept
{
    if (m_onFinished)
    {
        m_onFinished();
    }
}

} // namespace DragonOS::Animation
