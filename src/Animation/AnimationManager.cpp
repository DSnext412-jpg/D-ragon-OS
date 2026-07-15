/**
 * @file    AnimationManager.cpp
 * @brief   AnimationManager implementation.
 */

#include <Animation/AnimationManager.hpp>

namespace DragonOS::Animation {

void AnimationManager::Play(std::unique_ptr<Animation> animation) noexcept
{
    animation->Play();
    m_timeline.AddAnimation(std::move(animation));
}

void AnimationManager::Pause(Animation* animation) noexcept
{
    if (animation)
    {
        animation->Pause();
    }
}

void AnimationManager::Stop(Animation* animation) noexcept
{
    if (animation)
    {
        animation->Stop();
        m_timeline.RemoveAnimation(animation);
    }
}

void AnimationManager::Remove(Animation* animation) noexcept
{
    m_timeline.RemoveAnimation(animation);
}

void AnimationManager::Clear() noexcept
{
    m_timeline.Clear();
}

} // namespace DragonOS::Animation
