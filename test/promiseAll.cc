// SPDX-License-Identifier: MulanPSL-2.0
// Mostly by Claude 4.5 Opus.

#include <cassert>
#include <chrono>
#include <print>

#include <vega/Scheduler.h>
#include <vega/Promise.h>
#include <vega/PromiseAll.h>

using namespace vega;


// Test 1: Promise.all with homogeneous Promise<int>
Promise<> testHomogeneousPromises() {
    std::println("Test 1: Homogeneous Promise<int>...");
    
    auto p1 = Promise<int>::resolve(1);
    auto p2 = Promise<int>::resolve(2);
    auto p3 = Promise<int>::resolve(3);
    
    auto result = co_await promiseAll(p1, p2, p3);
    
    assert(result.size() == 3);
    assert(result[0] == 1);
    assert(result[1] == 2);
    assert(result[2] == 3);
    
    std::println("  PASSED: Got vector with values [1, 2, 3]");
}


// Test 2: Promise.all with callables returning same type
Promise<> testHomogeneousCallables() {
    std::println("Test 2: Homogeneous callables returning int...");
    
    auto f1 = []() { return 10; };
    auto f2 = []() { return 20; };
    auto f3 = []() { return 30; };
    
    auto result = co_await promiseAll(f1, f2, f3);
    
    assert(result.size() == 3);
    assert(result[0] == 10);
    assert(result[1] == 20);
    assert(result[2] == 30);
    
    std::println("  PASSED: Got vector with values [10, 20, 30]");
}


// Test 3: Promise.all with mixed Promise<int> and callables returning int
Promise<> testMixedHomogeneous() {
    std::println("Test 3: Mixed Promise<int> and callables returning int...");
    
    auto p1 = Promise<int>::resolve(100);
    auto f1 = []() { return 200; };
    auto p2 = Promise<int>::resolve(300);
    
    auto result = co_await promiseAll(p1, f1, p2);
    
    assert(result.size() == 3);
    assert(result[0] == 100);
    assert(result[1] == 200);
    assert(result[2] == 300);
    
    std::println("  PASSED: Got vector with values [100, 200, 300]");
}


// Test 4: Promise.all with callables returning Promise<int>
Promise<> testCallablesReturningPromise() {
    std::println("Test 4: Callables returning Promise<int>...");
    
    auto f1 = []() { return Promise<int>::resolve(1000); };
    auto f2 = []() { return Promise<int>::resolve(2000); };
    
    auto result = co_await promiseAll(f1, f2);
    
    assert(result.size() == 2);
    assert(result[0] == 1000);
    assert(result[1] == 2000);
    
    std::println("  PASSED: Got vector with values [1000, 2000]");
}


// Test 5: Promise.all with heterogeneous types (should return Promise<void>)
Promise<> testHeterogeneousTypes() {
    std::println("Test 5: Heterogeneous types (int and double)...");
    
    auto p1 = Promise<int>::resolve(42);
    auto p2 = Promise<double>::resolve(3.14);
    
    // This should return Promise<void>
    co_await promiseAll(p1, p2);

    std::println("  PASSED: Promise<void> resolved successfully");
}


// Test 6: Promise.all with async promises
Promise<> testAsyncPromises() {
    std::println("Test 6: Async promises with delay...");
    
    auto makeDelayedPromise = [](int value, int delayMs) -> Promise<int> {
        co_await Scheduler::getCurrent()->delay(std::chrono::milliseconds(delayMs));
        co_return value;
    };
    
    auto startTime = std::chrono::steady_clock::now();
    
    auto p1 = makeDelayedPromise(1, 100);
    auto p2 = makeDelayedPromise(2, 50);
    auto p3 = makeDelayedPromise(3, 150);
    
    auto result = co_await promiseAll(p1, p2, p3);
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    assert(result.size() == 3);
    assert(result[0] == 1);
    assert(result[1] == 2);
    assert(result[2] == 3);
    
    // Should take roughly 150ms (max of all delays), not 300ms (sum)
    assert(duration.count() >= 140 && duration.count() < 250);
    
    std::println("  PASSED: Got vector [1, 2, 3] in {}ms (parallel execution)", duration.count());
}


// Test 7: Promise.all with rejection
Promise<> testRejection() {
    std::println("Test 7: Promise rejection handling...");
    
    auto p1 = Promise<int>::resolve(1);
    auto p2 = Promise<int>::reject(std::runtime_error("Test error"));
    auto p3 = Promise<int>::resolve(3);
    
    bool caught = false;
    try {
        co_await promiseAll(p1, p2, p3);
    } catch (const std::runtime_error& e) {
        caught = true;
        assert(std::string(e.what()) == "Test error");
    }
    
    assert(caught);
    std::println("  PASSED: Rejection properly propagated");
}


// Test 8: Empty Promise.all
Promise<> testEmpty() {
    std::println("Test 8: Empty promiseAll...");
    
    co_await promiseAll();
    
    std::println("  PASSED: Empty promiseAll resolved to void");
}


// Test 9: Promise<void> types
Promise<> testVoidPromises() {
    std::println("Test 9: Promise<void> types...");
    
    bool executed1 = false;
    bool executed2 = false;
    
    auto p1 = Promise<void>::create([&executed1](auto resolve, auto) {
        executed1 = true;
        resolve();
    });
    
    auto p2 = Promise<void>::create([&executed2](auto resolve, auto) {
        executed2 = true;
        resolve();
    });
    
    co_await promiseAll(p1, p2);
    
    assert(executed1);
    assert(executed2);
    
    std::println("  PASSED: Both void promises executed");
}


// Test 10: Mixed callables - some return Promise<int>, some return int
Promise<> testMixedCallablesWithPromises() {
    std::println("Test 10: Mixed callables (returning int and Promise<int>)...");
    
    auto f1 = []() { return 5; };
    auto f2 = []() { return Promise<int>::resolve(10); };
    auto f3 = []() { return 15; };
    
    auto result = co_await promiseAll(f1, f2, f3);
    
    assert(result.size() == 3);
    assert(result[0] == 5);
    assert(result[1] == 10);
    assert(result[2] == 15);
    
    std::println("  PASSED: Got vector with values [5, 10, 15]");
}


Promise<> runAllTests() {
    std::println("Running Promise.all tests...\n");
    
    co_await testHomogeneousPromises();
    co_await testHomogeneousCallables();
    co_await testMixedHomogeneous();
    co_await testCallablesReturningPromise();
    co_await testHeterogeneousTypes();
    co_await testAsyncPromises();
    co_await testRejection();
    co_await testEmpty();
    co_await testVoidPromises();
    co_await testMixedCallablesWithPromises();
    
    std::println("\nAll tests passed!");
}


int main() {
    Scheduler::getDefault().runBlocking(runAllTests);
    return 0;
}


/*

Prompt (to Claude 4.5 Opus):

look my vega coroutine framework by learning @meson.build  first, and read @vega  framework code, finally @test  test-cases so you know how this framework works like javascript's promise. 

now, you have to implement Promise.all , in which if all passed with same return type, like all returning Promise<int>, your promise.all should create a Promise<int[]>. and if passed different , you should return Promise<void> since there is no great any type in c++

also, you'd know that all value can either be Promise that can be awaited, or maybe just standard function.
if passing promises and functions returning same type like passing functions return int and promise<int>s, the promise all should still return a promise that outs int array; and if passed something like function returning double with promises outs int, promise.all should return a promise that returns void.

the promise returned by promise.all should be added to the scheduler.

*/
