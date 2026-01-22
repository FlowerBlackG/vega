// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <vector>
#include <print>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

const int N_PROMISES = 6;
const long TIME_BETWEEN_MS = 100;

vega::Promise<void> suspendMain() {
    std::array<int, N_PROMISES> results;
    std::vector<vega::Promise<void>> promises;
    
    int seq = 0;

    for (int i = 0; i < N_PROMISES; i++) {
        auto promise = vega::Scheduler::get().setTimeout([i, &results, &seq] () {

            results[i] = ++seq;
        
        }, std::chrono::milliseconds(N_PROMISES * TIME_BETWEEN_MS - i * TIME_BETWEEN_MS));

        promises.push_back(std::move(promise));
    }

    for (auto& promise : promises) {
        co_await promise;
    }

    for (int i = 0; i < N_PROMISES; i++) {
        auto expected = N_PROMISES - i;
        auto actual = results[i];
        std::println("results[{}] should be {}. Actually: {}. {}", i, expected, actual, (expected == actual ? "OK" : "FAILED"));
        assert(expected == actual);
    }

    co_return;
}


int main() {
    vega::Scheduler::get().runBlocking(suspendMain);
    return 0;
}
