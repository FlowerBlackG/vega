// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <chrono>
#include <thread>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

using namespace vega;


// Use global scheduler.
Promise<> fluentMain() {
    co_await Promise<>::resolve();
    auto p = Promise<>();
    co_await Scheduler::get().delay(std::chrono::milliseconds(200));
}


// Use local scheduler.
Promise<> blockedMain() {
    co_await Promise<>();
}


int main() {
    enum class TaskStatus {
        Pending,
        Submitted,
        Finished,
    };

    TaskStatus fluentStatus = TaskStatus::Pending;
    TaskStatus blockedStatus = TaskStatus::Pending;
    
    auto fluentThread = std::thread([&fluentStatus] () {
        fluentStatus = TaskStatus::Submitted;
        Scheduler::get().runBlocking(fluentMain);
        fluentStatus = TaskStatus::Finished;
    });

    auto blockedThread = std::thread([&blockedStatus] () {
        blockedStatus = TaskStatus::Submitted;
        Scheduler{}.runBlocking(blockedMain);
        blockedStatus = TaskStatus::Finished;
    });


    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    assert(fluentStatus == TaskStatus::Finished);
    assert(blockedStatus == TaskStatus::Submitted);

    fluentThread.join();
    blockedThread.detach();

    return 0;
}
