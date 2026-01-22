// SPDX-License-Identifier: MulanPSL-2.0

#include <vega/Scheduler.h>

namespace vega {


size_t Scheduler::resolveDelayedTasks() {
    auto now = std::chrono::steady_clock::now();

    size_t count = 0;

    while (!delayedTasks.empty() && delayedTasks.top().resolveTime <= now) {
        auto task = delayedTasks.top();
        delayedTasks.pop();
        task.state->resolve();
        count++;
    }

    return count;
}
    

}  // namespace vega
