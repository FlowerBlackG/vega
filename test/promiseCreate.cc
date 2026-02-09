// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <chrono>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

using namespace vega;


Promise<> suspendMain() {
    bool mark = false;
    try {
        co_await Promise<>::create([](auto resolve, auto reject) -> Promise<> {
            co_await Scheduler::getCurrent().delay(std::chrono::milliseconds(20));
            reject(std::runtime_error("suspendMain"));
            resolve();
        });  // this should throw.

        assert(false);
    } catch (const std::exception& e) {
        mark = true;
    }

    assert(mark);
    mark = false;
    
    try {
        co_await Promise<>::create([](auto resolve, auto reject) -> Promise<> {
            co_await Scheduler::getCurrent().delay(std::chrono::milliseconds(20));
            resolve();
            reject(std::runtime_error("suspendMain"));
        });  // this should not throw.
    } catch (const std::exception& e) {
        assert(false);
    }

    
    try {
        Promise<>::create([](auto resolve, auto reject) -> Promise<> {
            co_await Scheduler::getCurrent().delay(std::chrono::milliseconds(20));
            reject(std::runtime_error("suspendMain"));
            resolve();
        });  // this promise throws, but ignored.
    } catch (const std::exception& e) {
        assert(false);
    }
    
    co_return;
}


int main() {
    Scheduler::getDefault().runBlocking(suspendMain);
    return 0;
}
