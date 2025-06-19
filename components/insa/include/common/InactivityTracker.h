#pragma once

#include <cstdint>
#include <functional>

class InactivityTracker
{
  public:
    InactivityTracker(uint64_t timeoutMs, std::function<void()> onTimeout);

    void update(uint64_t dt);
    void reset();
    void setEnabled(bool enabled);

  private:
    uint64_t m_timeoutMs;
    uint64_t m_elapsedTime;
    bool m_enabled;
    std::function<void()> m_onTimeout;
};
