// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <functional>
#include <memory>
#include <chrono>
#include <queue>
#include <vector>
#include <thread>


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


    /**
     *
     * 
     * @return size_t N-tasks resolved. 
     */
    size_t resolveDelayedTasks();

    /**
     *
     * 
     * @return size_t N-tasks dispatched. 
     */
    size_t dispatch() {
        size_t dispatched = 0;
        while (!regularTasks.empty() || !delayedTasks.empty()) {
            
            dispatched += resolveDelayedTasks();

            while (!regularTasks.empty()) {
                auto task = std::move(regularTasks.front());
                regularTasks.pop();
                task();
                dispatched++;
            }
        }
        
        return dispatched;
    }

public:
    static Scheduler& getInstance() { static Scheduler instance; return instance; }
    /** alias of getInstance(). */
    static Scheduler& get() { return getInstance(); }


    template<typename Callable>
    void runBlocking(Callable&& callable) {
        
        regularTasks.emplace( [&callable] () { callable(); } );

        while (!regularTasks.empty() || !delayedTasks.empty()) {
            size_t dispatched = dispatch();

            if (dispatched == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }


    template<typename _Rep, typename _Period>
    Promise<void> delay(const std::chrono::duration<_Rep, _Period>& duration) {
        Promise<void> ret;

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

};


}  // namespace vega
