// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <chrono>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

using namespace vega;

static bool suspendMainCompleted = false;

Promise<> throws() {
    throw std::runtime_error("throws");
    co_return;  // make this a suspendable function.
}


Promise<> throwInThrow(bool await) {
    if (await) {
        co_await throws();
    } else {
        throws();
    }
}


Promise<> suspendMain() {
    try {
        throws();
        co_await Scheduler::getCurrent().delay(std::chrono::milliseconds(20));
    } 
    catch (...) {
        assert(false);
    }


    try {
        co_await throws();
        assert(false);
    } 
    catch (const std::exception& e) {
        // ok
    }

    try {
        throwInThrow(true);
        throwInThrow(false);
        co_await throwInThrow(false);
    } 
    catch (...) {
        assert(false);
    }


    try {
        co_await throwInThrow(true);
        assert(false);
    } 
    catch (const std::exception& e) {
        // ok
    }


    suspendMainCompleted = true;
    co_return;
}


int main() {
    Scheduler::getDefault().runBlocking(suspendMain);
    assert(suspendMainCompleted);
    return 0;
}
