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

    while (true) {
        std::optional<DelayedTask> task;

        delayedTasks.withLock([&now, &task] (auto& it) {
            if (!it.empty() && it.top().resolveTime <= now) {
                task = it.top();
                it.pop();
            }
        });

        if (!task) {
            break;
        }

        task->state->resolve();
        count++;
    }

    return count;
}
    

size_t Scheduler::dispatchRegularTasks() {
    size_t count = 0;


    while (true) {
        std::optional<Scheduler::Task> task;

        regularTasks.withLock([&task] (auto& it) {
            if (!it.empty()) {
                task = std::move(it.front());
                it.pop();
            }
        });

        if (!task) {
            break;
        }

        task.value()();
        count++;
    }

    return count;
}


size_t Scheduler::removeCompletedTrackedPromises() {
    auto toBeRemoved = std::unordered_set<std::shared_ptr<PromiseStateBase>>();

    return trackedPromises.withLock([&toBeRemoved] (auto& promises) {
        for (auto it : promises) {
            if (it->status != PromiseStatus::Pending) {
                toBeRemoved.insert(it);
            }
        }

        for (auto it : toBeRemoved) {
            promises.erase(it);
        }

        return toBeRemoved.size();
    });
}
    

}  // namespace vega
