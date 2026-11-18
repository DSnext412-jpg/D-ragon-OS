

#pragma once

#include <Animation/AnimationCurve.hpp>
#include <Animation/AnimationState.hpp>
#include <Animation/Easing.hpp>

#include <functional>

namespace DragonOS::Animation {

class Animation {
public:
    using Callback = std::function<void()>;

    explicit Animation(
        float           duration,
        AnimationCurve  curve = AnimationCurve::Linear) noexcept;

    virtual ~Animation() noexcept = default;

    Animation(const Animation&)            = delete;
    Animation& operator=(const Animation&) = delete;
    Animation(Animation&&)                 = delete;
    Animation& operator=(Animation&&)      = delete;

    void Play() noexcept;

    void Pause() noexcept;

    void Stop() noexcept;

    void Reset() noexcept;
    void Update(float deltaTime) noexcept;


    [[nodiscard]] float               GetDuration()    const noexcept { return m_duration; }
    [[nodiscard]] float               GetElapsedTime() const noexcept { return m_elapsed; }
    [[nodiscard]] float               GetProgress()    const noexcept;
    [[nodiscard]] float               GetEasedProgress() const noexcept;
    [[nodiscard]] AnimationState      GetState()       const noexcept { return m_state; }
    [[nodiscard]] bool                IsPlaying()     const noexcept { return m_state == AnimationState::Playing; }
    [[nodiscard]] bool                IsPaused()      const noexcept { return m_state == AnimationState::Paused; }
    [[nodiscard]] bool                IsFinished()    const noexcept { return m_state == AnimationState::Finished; }
    [[nodiscard]] bool                GetLoop()       const noexcept { return m_loop; }
    [[nodiscard]] bool                GetReverse()    const noexcept { return m_reverse; }

    // ── Configuration ────────────────────────────────────────────────────

    void SetDuration(float duration) noexcept { m_duration = duration; }

    void SetLoop(bool loop) noexcept { m_loop = loop; }

    void SetReverse(bool reverse) noexcept { m_reverse = reverse; }

    void SetCurve(AnimationCurve curve) noexcept { m_curve = curve; }

    void SetOnFinished(Callback callback) noexcept { m_onFinished = std::move(callback); }

protected:
 
    virtual void OnUpdate(float progress) noexcept = 0;

    virtual void OnFinished() noexcept;

private:
    float           m_duration;
    float           m_elapsed{ 0.0f };
    AnimationState  m_state{ AnimationState::Idle };
    AnimationCurve  m_curve;
    bool            m_loop{ false };
    bool            m_reverse{ false };
    Callback        m_onFinished;
};

} 
