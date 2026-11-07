#ifndef _ZMC_TIMER_MGR_HPP_
#define _ZMC_TIMER_MGR_HPP_
#include <deque>

namespace zmc {
// Simplified Timer class
struct Timer {
    int id = 0;         // Timer ID (even for delay, odd for duration)
    uint32_t tick = 0;  // Target trigger time
    void runCycle() {}
};

// Simplified Timer Manager
template <typename T>
class TimerMgr {
   public:
    std::deque<T> timers;
    uint32_t tick = 0;

    void push(T tim) { timers.push_back(tim); }

    void runCycle() {
        tick++;
        for (auto it = timers.begin(); it != timers.end(); ++it) {
            if (it->tick <= tick) {
                it->runCycle();
                it = timers.erase(it);  // Remove completed timer
            }
        }
    }

    void clear() { timers.clear(); }

    void clear(int id) {
        for (auto it = timers.begin(); it != timers.end(); ++it) {
            if (it->id == id) {
                it = timers.erase(it);
                break;
            }
        }
    }

    // Clear all timers for a specific group ID
    void clearById(int tid) {
        for (auto it = timers.begin(); it != timers.end(); ++it) {
            if (it->id == tid) {
                it = timers.erase(it);
            }
        }
    }
    bool hasActiveTimer(int tid) const {
        for (const auto& timer : timers) {
            if (timer.id == tid) {
                return true;
            }
        }
        return false;
    }
};

}  // namespace zmc
#endif