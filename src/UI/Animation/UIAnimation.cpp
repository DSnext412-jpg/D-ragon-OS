#include <UI/Animation/UIAnimation.hpp>
#include <algorithm>

namespace DragonOS::UI::Animation {

UIAnimation::UIAnimation(UIElement* target, AnimatedProperty prop) noexcept
    : m_target(target)
    , m_property(prop)
{
}

void UIAnimation::Start() noexcept
{
    m_running = true;
    m_elapsed = 0;
    m_completed = false;
    m_paused = false;

    switch (m_property.type)
    {
    case AnimatedProperty::Opacity:
        m_target->SetAnimatedOpacity(m_property.from);
        break;
    case AnimatedProperty::OffsetX:
        m_target->SetAnimatedOffset(m_property.from, m_target->GetAnimatedOffsetY());
        break;
    case AnimatedProperty::OffsetY:
        m_target->SetAnimatedOffset(m_target->GetAnimatedOffsetX(), m_property.from);
        break;
    case AnimatedProperty::Scale:
        m_target->SetAnimatedScale(m_property.from);
        break;
    }
}

void UIAnimation::Stop() noexcept
{
    m_running = false;
}

void UIAnimation::Pause() noexcept
{
    m_paused = true;
}

void UIAnimation::Resume() noexcept
{
    m_paused = false;
}

float UIAnimation::ApplyEasing(float t) const noexcept
{
    switch (m_property.easing)
    {
    case Easing::Linear:
        return t;
    case Easing::EaseIn:
        return t * t;
    case Easing::EaseOut:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case Easing::EaseInOut:
    {
        float v = -2.0f * t + 2.0f;
        return t < 0.5f
            ? 2.0f * t * t
            : 1.0f - (v * v) / 2.0f;
    }
    case Easing::BounceOut:
    {
        if (t < 1.0f / 2.75f)
            return 7.5625f * t * t;
        else if (t < 2.0f / 2.75f)
        {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        }
        else if (t < 2.5f / 2.75f)
        {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        }
        else
        {
            t -= 2.625f / 2.75f;
            return 7.5625f * t * t + 0.984375f;
        }
    }
    }
    return t;
}

void UIAnimation::Update(float deltaTime) noexcept
{
    if (!m_running || m_paused || m_completed)
        return;

    m_elapsed += deltaTime;
    float t = (std::min)(m_elapsed / m_property.duration, 1.0f);
    float eased = ApplyEasing(t);
    float value = m_property.from + (m_property.to - m_property.from) * eased;

    switch (m_property.type)
    {
    case AnimatedProperty::Opacity:
        m_target->SetAnimatedOpacity(value);
        break;
    case AnimatedProperty::OffsetX:
        m_target->SetAnimatedOffset(value, m_target->GetAnimatedOffsetY());
        break;
    case AnimatedProperty::OffsetY:
        m_target->SetAnimatedOffset(m_target->GetAnimatedOffsetX(), value);
        break;
    case AnimatedProperty::Scale:
        m_target->SetAnimatedScale(value);
        break;
    }

    if (t >= 1.0f)
    {
        m_completed = true;
        m_running = false;
        if (m_onComplete)
            m_onComplete();
    }
}

UIAnimation* TransitionManager::Animate(UIElement* target, AnimatedProperty prop) noexcept
{
    auto anim = std::make_unique<UIAnimation>(target, prop);
    auto* ptr = anim.get();
    m_animations.push_back(std::move(anim));
    ptr->Start();
    return ptr;
}

void TransitionManager::Update(float deltaTime) noexcept
{
    for (auto it = m_animations.begin(); it != m_animations.end(); )
    {
        (*it)->Update(deltaTime);
        if ((*it)->IsCompleted())
            it = m_animations.erase(it);
        else
            ++it;
    }
}

void TransitionManager::Clear() noexcept
{
    m_animations.clear();
}

void TransitionManager::StopAll() noexcept
{
    for (auto& anim : m_animations)
        anim->Stop();
    m_animations.clear();
}

void TransitionManager::FadeIn(UIElement* target, float duration) noexcept
{
    AnimatedProperty prop;
    prop.type = AnimatedProperty::Opacity;
    prop.from = 0.0f;
    prop.to = 1.0f;
    prop.duration = duration;
    prop.easing = Easing::EaseOut;
    Animate(target, prop);
}

void TransitionManager::FadeOut(UIElement* target, float duration) noexcept
{
    AnimatedProperty prop;
    prop.type = AnimatedProperty::Opacity;
    prop.from = target->GetAnimatedOpacity();
    prop.to = 0.0f;
    prop.duration = duration;
    prop.easing = Easing::EaseOut;
    Animate(target, prop);
}

void TransitionManager::SlideIn(UIElement* target, float fromX, float fromY, float duration) noexcept
{
    AnimatedProperty px;
    px.type = AnimatedProperty::OffsetX;
    px.from = fromX;
    px.to = 0.0f;
    px.duration = duration;
    px.easing = Easing::EaseOut;
    Animate(target, px);

    AnimatedProperty py;
    py.type = AnimatedProperty::OffsetY;
    py.from = fromY;
    py.to = 0.0f;
    py.duration = duration;
    py.easing = Easing::EaseOut;
    Animate(target, py);
}

void TransitionManager::ScaleIn(UIElement* target, float duration) noexcept
{
    AnimatedProperty prop;
    prop.type = AnimatedProperty::Scale;
    prop.from = 0.0f;
    prop.to = 1.0f;
    prop.duration = duration;
    prop.easing = Easing::BounceOut;
    Animate(target, prop);
}

void TransitionManager::AnimateStateTransition(UIElement* target, ElementState oldState, ElementState newState) noexcept
{
    if (oldState == newState) return;

    if (oldState == ElementState::Normal && newState == ElementState::Hover)
    {
        AnimatedProperty s;
        s.type = AnimatedProperty::Scale;
        s.from = 1.0f;
        s.to = 1.05f;
        s.duration = 0.2f;
        s.easing = Easing::EaseOut;
        Animate(target, s);
    }
    else if (oldState == ElementState::Hover && newState == ElementState::Normal)
    {
        AnimatedProperty s;
        s.type = AnimatedProperty::Scale;
        s.from = 1.05f;
        s.to = 1.0f;
        s.duration = 0.2f;
        s.easing = Easing::EaseOut;
        Animate(target, s);
    }
    else if (oldState == ElementState::Normal && newState == ElementState::Pressed)
    {
        AnimatedProperty s;
        s.type = AnimatedProperty::Scale;
        s.from = 1.0f;
        s.to = 0.95f;
        s.duration = 0.1f;
        s.easing = Easing::EaseIn;
        Animate(target, s);
    }
    else if (oldState == ElementState::Pressed && newState == ElementState::Normal)
    {
        AnimatedProperty s;
        s.type = AnimatedProperty::Scale;
        s.from = 0.95f;
        s.to = 1.0f;
        s.duration = 0.15f;
        s.easing = Easing::EaseOut;
        Animate(target, s);
    }
    else if (oldState == ElementState::Pressed && newState == ElementState::Hover)
    {
        AnimatedProperty s;
        s.type = AnimatedProperty::Scale;
        s.from = 0.95f;
        s.to = 1.05f;
        s.duration = 0.15f;
        s.easing = Easing::EaseOut;
        Animate(target, s);
    }
}

} // namespace
