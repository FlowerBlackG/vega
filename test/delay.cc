// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>

#include <vega/Scheduler.h>
#include <vega/Promise.h>


int main() {
    vega::Scheduler::get().runBlocking([] () -> vega::Promise<void> {

        auto t0 = std::chrono::steady_clock::now();

        // delay but fire and forget
        vega::Scheduler::get().delay(std::chrono::milliseconds(1000));

        auto t1 = std::chrono::steady_clock::now();

        // delay and wait
        co_await vega::Scheduler::get().delay(std::chrono::milliseconds(2000));

        auto t2 = std::chrono::steady_clock::now();

        // delay but fire and forget
        vega::Scheduler::get().delay(std::chrono::milliseconds(1000));

        auto t3 = std::chrono::steady_clock::now();

        assert(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() < 800);
        assert(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() >= 1990);
        assert(std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() < 800);

        co_return;
    });

    return 0;
}
