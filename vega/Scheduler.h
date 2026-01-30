// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <functional>
#include <memory>
#include <chrono>
#include <queue>
#include <vector>
#include <thread>
#include <unordered_set>
#include <mutex>
#include <semaphore>
#include <atomic>

#include <vega/Promise.h>


namespace vega {


class Scheduler {
protected:
    using Task = std::function<void()>;

    struct DelayedTask {
        std::shared_ptr<PromiseState<void>> state;
        std::chrono::steady_clock::time_point resolveTime;
        auto operator <=> (const DelayedTask& other) const { return resolveTime <=> other.resolveTime; }
        bool ready() const { return resolveTime <= std::chrono::steady_clock::now(); }
    };

    template <typename T>
    struct Synchronized {
        T data;
        std::mutex lock;

        bool empty() {
            const std::lock_guard<std::mutex> _l {lock};
            return data.empty();
        }

        template <typename F>
        auto withLock(F&& f) {
            const std::lock_guard<std::mutex> _l {lock};
            return f(data);
        }
    };

    /* -------- tasks -------- */

    Synchronized<std::queue<Task>> regularTasks;

    Synchronized<
        std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<DelayedTask>>
    > delayedTasks;
    
    Synchronized<std::unordered_set<std::shared_ptr<PromiseStateBase>>> trackedPromises;

    
    /* -------- workers -------- */
    
    /**
     * If set to 0, the scheduler will use no worker threads.
     *
     * Total threads used by a scheduler is nWorkers + 1 (scheduler's main thread).
     */
    const size_t nWorkers = 0;
    std::vector<std::thread> workerThreads;
    std::counting_semaphore<> taskSemaphore {0};
    std::atomic<bool> stopWorkers {false};
    std::atomic<bool> workersStarted {false};
    std::atomic<size_t> activeWorkers {0};

    /**
     *
     * 
     * @return size_t N-tasks resolved. 
     */
    size_t dispatchDelayedTasks();
    size_t dispatchRegularTasks();

    size_t removeCompletedTrackedPromises();

    void startWorkers();
    void stopAndJoinWorkers();
    void workerThreadMain(size_t workerId);

    /**
     * Dispatch tasks on scheduler's main thread.
     * 
     * @return size_t N-tasks dispatched. 
     */
    size_t dispatch();

    bool hasPendingTasks();


    template <typename _Rep, typename _Period>
    void drain(std::chrono::duration<_Rep, _Period> snap) {
        while (this->hasPendingTasks()) {
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
    Scheduler(size_t nWorkers = 0);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator = (const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator = (Scheduler&&) = delete;


    /**
     * Get the global single-threaded scheduler.
     */
    static Scheduler& getInstance() { static Scheduler instance {0}; return instance; }
    /** alias of getInstance(). */
    static Scheduler& get() { return getInstance(); }


    /**
     * Get the current scheduler of the calling coroutine (also it is the scheduler using the thread).
     * @return The current scheduler if running inside a scheduler context, nullptr otherwise.
     */
    static Scheduler* getCurrent();

    /**
     * Check if the current thread is a worker thread of this scheduler.
     */
    bool isCurrentThreadWorker() const;

    /**
     * Check if the current thread is the main scheduler thread.
     */
    bool isCurrentThreadMain() const;


    void track(std::shared_ptr<PromiseStateBase> promise) {
        trackedPromises.withLock([&promise] (auto& it) {
            it.emplace(std::move(promise));
        });
    }


    template <typename T>
    void track(const Promise<T>& promise) {
        this->track(promise.state);
    }


    template<typename F>
    requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, Promise<void>>
    void runBlocking(F&& callable) {
        Scheduler* previousScheduler = Scheduler::setCurrent(this);

        auto promise = callable();

        trackedPromises.withLock([&promise] (auto& it) {
            it.emplace(promise.state);
        });

        drain();

        Scheduler::setCurrent(previousScheduler);
    }


    template<typename F>
    requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, void>
    void runBlocking(F&& callable) {
        Scheduler* previousScheduler = Scheduler::setCurrent(this);

        this->addTask([&callable] () { callable(); });

        drain();

        Scheduler::setCurrent(previousScheduler);
    }



    template<typename _Rep, typename _Period>
    Promise<void> delay(const std::chrono::duration<_Rep, _Period>& duration) {
        Promise<void> ret;
        ret.state->scheduler = this;

        std::chrono::steady_clock::time_point resolveTime = std::chrono::steady_clock::now() + duration;
        
        delayedTasks.withLock([&ret, resolveTime] (auto& it) {
            it.push({
                .state = ret.state,
                .resolveTime = resolveTime,
            });
        });

        return ret;
    }


    template<typename Func, typename _Rep, typename _Period>
    Promise<void> setTimeout(Func func, const std::chrono::duration<_Rep, _Period>& duration) {
        co_await this->delay(duration);
        func();
    }


    void addTask(Task task) {
        regularTasks.withLock([&task] (auto& it) {
            it.emplace(std::move(task));
        });
        
        if (workersStarted)
            taskSemaphore.release();
    }

    bool shouldQueueTask() const;

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
