/**
 * @file    AnimationTimeline.cpp
 * @brief   AnimationTimeline implementation.
 */

#include <Animation/AnimationTimeline.hpp>

#include <algorithm>

namespace DragonOS::Animation {

void AnimationTimeline::AddAnimation(std::unique_ptr<Animation> animation) noexcept
{
    if (animation)
    {
        m_animations.push_back(std::move(animation));
    }
}

bool AnimationTimeline::RemoveAnimation(Animation* animation) noexcept
{
    if (!animation) { return false; }

    const auto it = std::find_if(
        m_animations.begin(), m_animations.end(),
        [animation](const auto& ptr) { return ptr.get() == animation; });

    if (it == m_animations.end()) { return false; }

    m_animations.erase(it);
    return true;
}

void AnimationTimeline::Clear() noexcept
{
    m_animations.clear();
}

void AnimationTimeline::Update(float deltaTime) noexcept
{
    // Update every animation.
    for (auto& anim : m_animations)
    {
        if (anim)
        {
            anim->Update(deltaTime);
        }
    }

    // Remove finished non-looping animations.
    m_animations.erase(
        std::remove_if(
            m_animations.begin(), m_animations.end(),
            [](const auto& ptr)
            {
                return ptr && ptr->IsFinished() && !ptr->GetLoop();
            }),
        m_animations.end());
}

} // namespace DragonOS::Animation
