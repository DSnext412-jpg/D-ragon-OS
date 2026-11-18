
#pragma once

#include <Animation/Animation.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace DragonOS::Animation {

class AnimationTimeline final {
public:
    AnimationTimeline() noexcept = default;
    ~AnimationTimeline() noexcept = default;

    AnimationTimeline(const AnimationTimeline&)            = delete;
    AnimationTimeline& operator=(const AnimationTimeline&) = delete;
    AnimationTimeline(AnimationTimeline&&)                 = delete;
    AnimationTimeline& operator=(AnimationTimeline&&)      = delete;

    void AddAnimation(std::unique_ptr<Animation> animation) noexcept;

  
    bool RemoveAnimation(Animation* animation) noexcept;

    void Clear() noexcept;

    void Update(float deltaTime) noexcept;

    [[nodiscard]] std::size_t Count() const noexcept { return m_animations.size(); }

    [[nodiscard]] bool Empty() const noexcept { return m_animations.empty(); }

private:
    std::vector<std::unique_ptr<Animation>> m_animations;
};

}
