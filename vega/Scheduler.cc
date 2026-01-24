// SPDX-License-Identifier: MulanPSL-2.0

#include <vega/Scheduler.h>

namespace vega {


/**
 * It is user's responsibility (currently) to ensure structured concurrency.
 */
static thread_local Scheduler* currentScheduler = nullptr;


Scheduler* Scheduler::getCurrent() {
    return currentScheduler;
}


Scheduler* Scheduler::setCurrent(Scheduler* scheduler) {
    auto prev = currentScheduler;
    currentScheduler = scheduler;
    return prev;
}


size_t Scheduler::dispatchDelayedTasks() {
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
    

size_t Scheduler::dispatchRegularTasks() {
    size_t count = 0;

    while (!regularTasks.empty()) {
        auto task = std::move(regularTasks.front());
        regularTasks.pop();
        task();
        count++;
    }

    return count;
}


size_t Scheduler::removeCompletedTrackedPromises() {
    auto toBeRemoved = std::unordered_set<std::shared_ptr<PromiseStateBase>>();

    for (auto it : trackedPromises) {
        if (it->status != PromiseStatus::Pending) {
            toBeRemoved.insert(it);
        }
    }

    size_t ret = toBeRemoved.size();

    for (auto it : toBeRemoved) {
        trackedPromises.erase(it);
    }

    return ret;
}
    

}  // namespace vega
