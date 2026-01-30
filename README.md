# Vega C++ Coroutine Framework

A modern, lightweight C++ coroutine framework featuring TypeScript-like Promise API, multi-threading support, and more.

## CI

[![Meson Test](https://github.com/FlowerBlackG/vega/actions/workflows/meson-test.yml/badge.svg)](https://github.com/FlowerBlackG/vega/actions/workflows/meson-test.yml)

## Vega Features

✅️ TypeScript-like Promise style API.
✅️ Multi-threaded execution.
✅️ io_uring-based file operations (falls back to fstream if not supported).
🚧 io_uring-based networking.

## Packaging

✅️ Copy the whole `vega` folder to your project.
🚧 Pre-compiled libvega.so and vega.dll.
🚧 Well-documented and organized headers.

## Examples

**setTimeout**

```cpp
#include <chrono>
#include <print>
#include <vega/vega.h>

using namespace vega;

static inline void printTime(const std::string& msg) {
    auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::println("{}: {}", std::format("{:%T}", now), msg);
}

Promise<> suspendMain() {
    
    printTime("entry");

    auto timeout = Scheduler::get().setTimeout([] () {
    
        printTime("setTimeout callback");

    }, std::chrono::seconds(1));

    printTime("after setTimeout");

    co_await timeout;

    printTime("after await timeout");

    co_await Scheduler::get().delay(std::chrono::seconds(1));

    printTime("after delay");
}

int main() {
    Scheduler::get().runBlocking(suspendMain);
    return 0;
}

```

Result:

```
13:57:04.607: entry
13:57:04.607: after setTimeout
13:57:05.608: setTimeout callback
13:57:05.608: after await timeout
13:57:06.608: after delay
```

---

**Multiple Workers**

```cpp
// SPDX-License-Identifier: MulanPSL-2.0

#include <chrono>
#include <print>
#include <vector>
#include <vega/vega.h>

using namespace vega;

Promise<> heavyTask() {
    co_await Scheduler::getCurrent()->delay(std::chrono::seconds(0));
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

Promise<> suspendMain() {
    std::vector<Promise<>> promises;
    for (int i = 0; i < 15; i++)
        promises.emplace_back(heavyTask());

    for (auto &promise : promises)
        co_await promise;
}

int main() {
    auto t1 = std::chrono::system_clock::now();
    Scheduler { 8 }.runBlocking(suspendMain);
    auto t2 = std::chrono::system_clock::now();
    auto duration = t2 - t1;
    std::println("Duration: {} ms.", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    return 0;
}

```

Result:

```
Duration: 2001 ms.
```
