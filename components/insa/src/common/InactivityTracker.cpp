#include "common/InactivityTracker.h"

InactivityTracker::InactivityTracker(uint64_t timeoutMs, std::function<void()> onTimeout)
    : m_timeoutMs(timeoutMs), m_elapsedTime(0), m_enabled(true), m_onTimeout(onTimeout)
{
}

void InactivityTracker::update(uint64_t dt)
{
    if (!m_enabled)
        return;

    m_elapsedTime += dt;

    if (m_elapsedTime >= m_timeoutMs && m_onTimeout)
    {
        m_onTimeout();
        m_enabled = false;
    }
}

void InactivityTracker::reset()
{
    m_elapsedTime = 0;
    m_enabled = true;
}

void InactivityTracker::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (enabled)
    {
        reset();
    }
}
