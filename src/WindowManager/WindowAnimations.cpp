#include <WindowManager/WindowAnimations.hpp>
#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::WindowManager {

namespace {

using AnimAnimation = ::DragonOS::Animation::Animation;
using AnimCurve     = ::DragonOS::Animation::AnimationCurve;

class WindowMoveAnimation final : public AnimAnimation {
public:
    WindowMoveAnimation(
        DragonWindow* window,
        float targetX, float targetY,
        float duration,
        AnimCurve curve) noexcept
        : AnimAnimation{ duration, curve }
        , m_pWindow{ window }
        , m_startX{ window->GetX() }
        , m_startY{ window->GetY() }
        , m_endX{ targetX }
        , m_endY{ targetY }
    {}

protected:
    void OnUpdate(float progress) noexcept override
    {
        const float x = m_startX + (m_endX - m_startX) * progress;
        const float y = m_startY + (m_endY - m_startY) * progress;
        m_pWindow->Move(x, y);
    }

private:
    DragonWindow* m_pWindow;
    float m_startX, m_startY;
    float m_endX, m_endY;
};

class WindowResizeAnimation final : public AnimAnimation {
public:
    WindowResizeAnimation(
        DragonWindow* window,
        float targetW, float targetH,
        float duration,
        AnimCurve curve) noexcept
        : AnimAnimation{ duration, curve }
        , m_pWindow{ window }
        , m_startW{ window->GetWidth() }
        , m_startH{ window->GetHeight() }
        , m_endW{ targetW }
        , m_endH{ targetH }
    {}

protected:
    void OnUpdate(float progress) noexcept override
    {
        const float w = m_startW + (m_endW - m_startW) * progress;
        const float h = m_startH + (m_endH - m_startH) * progress;
        m_pWindow->Resize(w, h);
    }

private:
    DragonWindow* m_pWindow;
    float m_startW, m_startH;
    float m_endW, m_endH;
};

class WindowTransformAnimation final : public AnimAnimation {
public:
    WindowTransformAnimation(
        DragonWindow* window,
        float targetX, float targetY,
        float targetW, float targetH,
        float duration,
        AnimCurve curve) noexcept
        : AnimAnimation{ duration, curve }
        , m_pWindow{ window }
        , m_startX{ window->GetX() }
        , m_startY{ window->GetY() }
        , m_startW{ window->GetWidth() }
        , m_startH{ window->GetHeight() }
        , m_endX{ targetX }
        , m_endY{ targetY }
        , m_endW{ targetW }
        , m_endH{ targetH }
    {}

protected:
    void OnUpdate(float progress) noexcept override
    {
        const float x = m_startX + (m_endX - m_startX) * progress;
        const float y = m_startY + (m_endY - m_startY) * progress;
        const float w = m_startW + (m_endW - m_startW) * progress;
        const float h = m_startH + (m_endH - m_startH) * progress;
        m_pWindow->Move(x, y);
        m_pWindow->Resize(w, h);
    }

private:
    DragonWindow* m_pWindow;
    float m_startX, m_startY;
    float m_startW, m_startH;
    float m_endX, m_endY;
    float m_endW, m_endH;
};

} // anonymous namespace

std::unique_ptr<::DragonOS::Animation::Animation> CreateMoveAnimation(
    DragonWindow* window,
    float targetX, float targetY,
    float duration,
    ::DragonOS::Animation::AnimationCurve curve) noexcept
{
    return std::make_unique<WindowMoveAnimation>(window, targetX, targetY, duration, curve);
}

std::unique_ptr<::DragonOS::Animation::Animation> CreateResizeAnimation(
    DragonWindow* window,
    float targetW, float targetH,
    float duration,
    ::DragonOS::Animation::AnimationCurve curve) noexcept
{
    return std::make_unique<WindowResizeAnimation>(window, targetW, targetH, duration, curve);
}

std::unique_ptr<::DragonOS::Animation::Animation> CreateTransformAnimation(
    DragonWindow* window,
    float targetX, float targetY,
    float targetW, float targetH,
    float duration,
    ::DragonOS::Animation::AnimationCurve curve) noexcept
{
    return std::make_unique<WindowTransformAnimation>(window, targetX, targetY, targetW, targetH, duration, curve);
}

} // namespace DragonOS::WindowManager
