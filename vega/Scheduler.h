// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <functional>
#include <memory>
#include <chrono>
#include <queue>
#include <vector>
#include <thread>
#include <unordered_set>

#include <vega/Promise.h>


namespace vega {


class Scheduler {
private:
    using Task = std::function<void()>;

    struct DelayedTask {
        std::shared_ptr<PromiseState<void>> state;
        std::chrono::steady_clock::time_point resolveTime;
        auto operator <=> (const DelayedTask& other) const { return resolveTime <=> other.resolveTime; }
        bool ready() const { return resolveTime <= std::chrono::steady_clock::now(); }
    };

    std::queue<Task> regularTasks;
    std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<DelayedTask>> delayedTasks;
    std::unordered_set<std::shared_ptr<PromiseStateBase>> trackedPromises;

    /**
     *
     * 
     * @return size_t N-tasks resolved. 
     */
    size_t dispatchDelayedTasks();
    size_t dispatchRegularTasks();

    size_t removeCompletedTrackedPromises();

    /**
     *
     * 
     * @return size_t N-tasks dispatched. 
     */
    size_t dispatch() {
        size_t dispatched = 0;
        
        dispatched += dispatchDelayedTasks();
        dispatched += dispatchRegularTasks();
        
        return dispatched;
    }


    template <typename _Rep, typename _Period>
    void drain(std::chrono::duration<_Rep, _Period> snap) {
        while (!regularTasks.empty() || !delayedTasks.empty() || !trackedPromises.empty()) {
            size_t dispatched = dispatch();
            size_t removedPromises = removeCompletedTrackedPromises();

            if (dispatched + removedPromises == 0) {
                std::this_thread::sleep_for(snap);
            }
        }
    }

    void drain() { drain(std::chrono::microseconds(100)); }

    /**
     * Set thread's current scheduler as [scheduler].
     *
     * @return Previous scheduler.
     */
    static Scheduler* setCurrent(Scheduler* scheduler);
    static Scheduler* setCurrent(Scheduler& scheduler) { return Scheduler::setCurrent(&scheduler); }


public:
    static Scheduler& getInstance() { static Scheduler instance; return instance; }
    /** alias of getInstance(). */
    static Scheduler& get() { return getInstance(); }


    /**
     * Get the current scheduler of the calling coroutine (also it is the scheduler using the thread).
     * @return The current scheduler if running inside a scheduler context, nullptr otherwise.
     */
    static Scheduler* getCurrent();

    template<typename F>
    requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, Promise<void>>
    void runBlocking(F&& callable) {
        Scheduler* previousScheduler = Scheduler::setCurrent(this);

        trackedPromises.emplace(callable().state);
        drain();

        Scheduler::setCurrent(previousScheduler);
    }


    template<typename F>
    requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, void>
    void runBlocking(F&& callable) {
        Scheduler* previousScheduler = Scheduler::setCurrent(this);

        regularTasks.emplace([&callable] () { callable(); });
        drain();

        Scheduler::setCurrent(previousScheduler);
    }



    template<typename _Rep, typename _Period>
    Promise<void> delay(const std::chrono::duration<_Rep, _Period>& duration) {
        Promise<void> ret;
        ret.state->scheduler = this;

        std::chrono::steady_clock::time_point resolveTime = std::chrono::steady_clock::now() + duration;
        
        delayedTasks.push({
            .state = ret.state,
            .resolveTime = resolveTime,
        });

        return ret;
    }


    template<typename Func, typename _Rep, typename _Period>
    Promise<void> setTimeout(Func func, const std::chrono::duration<_Rep, _Period>& duration) {
        co_await this->delay(duration);
        func();
    }


    void addTask(Task task) {
        regularTasks.push(std::move(task));
    }



    friend Scheduler* setCurrentScheduler(Scheduler* scheduler);
    friend Scheduler* setCurrentScheduler(Scheduler& scheduler);
};


inline Scheduler* getCurrentScheduler() {
    return Scheduler::getCurrent();
}


inline Scheduler* setCurrentScheduler(Scheduler* scheduler) {
    return Scheduler::setCurrent(scheduler);
}


inline Scheduler* setCurrentScheduler(Scheduler& scheduler) {
    return Scheduler::setCurrent(scheduler);
}


}  // namespace vega
