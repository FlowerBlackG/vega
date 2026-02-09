// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <chrono>
#include <thread>
#include <functional>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

using namespace vega;

static std::thread::id nioThreadId;
static const auto NIO_JOB_DURATION = std::chrono::milliseconds(800);
static const int NIO_RESULT = 0x18961907;

struct Callbacks {
    std::function<void(int)> resume = nullptr;
    std::function<void(const std::runtime_error&)> resumeWithException = nullptr;

    std::mutex lock;
} static callbacks;


static bool suspendMainFinished = false;

static void assertNotInNioThread() {
    assert(std::this_thread::get_id() != nioThreadId);
}


Promise<> suspendMain() {
    assertNotInNioThread();

    auto time1 = std::chrono::steady_clock::now();

    auto successPromise = Promise<int>::create([](auto resolve, auto _) {
        std::lock_guard<std::mutex> _g {callbacks.lock};
        callbacks.resume = resolve;
        callbacks.resumeWithException = nullptr;
    });
    
    co_await Scheduler::getCurrent().delay(NIO_JOB_DURATION);

    auto result = co_await successPromise;

    auto time2 = std::chrono::steady_clock::now();

    assertNotInNioThread();
    assert(time2 - time1 >= NIO_JOB_DURATION && time2 - time1 < NIO_JOB_DURATION * 2);
    assert(result == NIO_RESULT);

    auto failPromise = Promise<>::create([](auto _, auto reject) {
        std::lock_guard<std::mutex> _g {callbacks.lock};
        callbacks.resume = nullptr;
        callbacks.resumeWithException = reject;
    });

    try {
        co_await failPromise;  // this should throw.
        assert(false);
    }
    catch (...) {
        // ok
    }

    assertNotInNioThread();
    suspendMainFinished = true;
}


void nioThreadMain() {
    while (!suspendMainFinished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        Callbacks c;
        
        {
            std::lock_guard<std::mutex> _g {callbacks.lock};
            c.resume = callbacks.resume;
            c.resumeWithException = callbacks.resumeWithException;

            callbacks.resume = nullptr;
            callbacks.resumeWithException = nullptr;
        }

        if (c.resume || c.resumeWithException)
            std::this_thread::sleep_for(std::chrono::milliseconds(NIO_JOB_DURATION));

        if (c.resume)
            c.resume(NIO_RESULT);
        if (c.resumeWithException)
            c.resumeWithException(std::runtime_error("nio"));
    }
}


int main() {
    auto nioThread = std::thread(nioThreadMain);
    nioThreadId = nioThread.get_id();

    Scheduler::getDefault().runBlocking(suspendMain);
    
    assertNotInNioThread();
    assert(suspendMainFinished);
    
    nioThread.join();
    return 0;
}
