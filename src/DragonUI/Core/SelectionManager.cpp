#include <DragonUI/Core/SelectionManager.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void SelectionManager::SetMode(SelectionMode mode) noexcept
{
    if (m_mode == mode)
        return;

    m_mode = mode;
    if (mode == SelectionMode::None)
        m_selected.clear();
    else if (mode == SelectionMode::Single && m_selected.size() > 1)
        m_selected.resize(1);

    NotifyChanged();
}

void SelectionManager::SetItemCount(int64_t count) noexcept
{
    if (count < 0)
        count = 0;

    m_itemCount = count;

    const auto isOutOfRange = [count](int64_t index) { return index < 0 || index >= count; };
    const auto end = std::remove_if(m_selected.begin(), m_selected.end(), isOutOfRange);
    if (end != m_selected.end())
    {
        m_selected.erase(end, m_selected.end());
        NotifyChanged();
    }

    if (m_anchor >= count) m_anchor = count > 0 ? count - 1 : -1;
    if (m_current >= count) m_current = count > 0 ? count - 1 : -1;
}

void SelectionManager::OnPointerClick(int64_t index, bool ctrl, bool shift) noexcept
{
    if (m_mode == SelectionMode::None || index < 0 || index >= m_itemCount)
        return;

    if (ctrl && shift)
    {
        if (m_anchor >= 0)
            SelectRange(m_anchor, index, /*additive*/ true);
        else
            SelectSingle(index);
        return;
    }

    if (ctrl)
    {
        Toggle(index);
        return;
    }

    if (shift)
    {
        if (m_anchor >= 0)
            SelectRange(m_anchor, index, /*additive*/ false);
        else if (m_current >= 0)
            SelectRange(m_current, index, /*additive*/ false);
        else
            SelectSingle(index);
        return;
    }

    SelectSingle(index);
}

void SelectionManager::OnKeyMove(int64_t index, bool ctrl, bool shift) noexcept
{
    if (m_mode == SelectionMode::None)
    {
        SetCurrent(index);
        return;
    }

    if (shift)
    {
        const int64_t anchor = m_anchor >= 0 ? m_anchor : m_current;
        if (anchor < 0)
        {
            SelectSingle(index);
        }
        else
        {
            // Track the live extent between the anchor and the moving
            // current item so Shift+arrows grow/shrink the range.
            const int64_t lo = (std::min)(anchor, index);
            const int64_t hi = (std::max)(anchor, index);

            std::vector<int64_t> range;
            range.reserve(static_cast<size_t>(hi - lo + 1));
            for (int64_t i = lo; i <= hi; ++i)
                range.push_back(i);

            m_suppressNotify = true;
            m_selected.swap(range);
            m_suppressNotify = false;
            SetCurrent(index);
            NotifyChanged();
        }
        return;
    }

    if (ctrl)
    {
        // Ctrl+arrow only moves the focus ring, does not alter selection.
        SetCurrent(index);
        return;
    }

    SelectSingle(index);
}

void SelectionManager::SelectSingle(int64_t index) noexcept
{
    if (m_mode == SelectionMode::None || index < 0 || index >= m_itemCount)
        return;

    m_selected.clear();
    m_selected.push_back(index);
    m_anchor = index;
    m_current = index;
    NotifyChanged();
}

void SelectionManager::Toggle(int64_t index) noexcept
{
    if (m_mode == SelectionMode::None || index < 0 || index >= m_itemCount)
        return;

    if (m_mode == SelectionMode::Single)
    {
        SelectSingle(index);
        return;
    }

    if (Contains(index))
    {
        const auto it = std::find(m_selected.begin(), m_selected.end(), index);
        if (it != m_selected.end())
            m_selected.erase(it);
    }
    else
    {
        InsertSorted(index);
    }

    m_anchor = index;
    m_current = index;
    NotifyChanged();
}

void SelectionManager::SelectRange(int64_t from, int64_t to, bool additive) noexcept
{
    if (m_mode == SelectionMode::None)
        return;

    const int64_t lo = (std::max<int64_t>)(0, (std::min)(from, to));
    const int64_t hi = (std::min)(m_itemCount - 1, (std::max)(from, to));
    if (hi < lo)
        return;

    if (!additive)
        m_selected.clear();

    for (int64_t i = lo; i <= hi; ++i)
        InsertSorted(i);

    if (m_mode == SelectionMode::Single)
    {
        m_selected.resize(1);
        m_selected[0] = to;
    }

    m_anchor = from;
    m_current = to;
    NotifyChanged();
}

void SelectionManager::SelectAll() noexcept
{
    if (m_mode == SelectionMode::None || m_itemCount == 0)
        return;

    m_selected.clear();
    m_selected.reserve(static_cast<size_t>(m_itemCount));
    for (int64_t i = 0; i < m_itemCount; ++i)
        m_selected.push_back(i);

    m_anchor = 0;
    m_current = m_itemCount - 1;
    NotifyChanged();
}

void SelectionManager::Clear() noexcept
{
    if (m_selected.empty())
    {
        m_current = -1;
        return;
    }

    m_selected.clear();
    NotifyChanged();
}

bool SelectionManager::IsSelected(int64_t index) const noexcept
{
    return Contains(index);
}

int64_t SelectionManager::GetFirstSelected() const noexcept
{
    return m_selected.empty() ? -1 : m_selected.front();
}

int64_t SelectionManager::GetLastSelected() const noexcept
{
    return m_selected.empty() ? -1 : m_selected.back();
}

void SelectionManager::SetAnchor(int64_t index) noexcept
{
    m_anchor = (index >= 0 && index < m_itemCount) ? index : -1;
}

void SelectionManager::SetCurrent(int64_t index) noexcept
{
    m_current = (index >= 0 && index < m_itemCount) ? index : -1;
}

bool SelectionManager::Contains(int64_t index) const noexcept
{
    return std::binary_search(m_selected.begin(), m_selected.end(), index);
}

void SelectionManager::InsertSorted(int64_t index) noexcept
{
    const auto it = std::lower_bound(m_selected.begin(), m_selected.end(), index);
    if (it != m_selected.end() && *it == index)
        return;
    m_selected.insert(it, index);
}

void SelectionManager::NotifyChanged() noexcept
{
    if (m_suppressNotify)
        return;
    if (m_onChanged)
        m_onChanged(*this);
}

} // namespace DragonOS::DragonUI
