// SPDX-License-Identifier: MulanPSL-2.0

#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <print>
#include <iostream>
#include <map>
#include <mutex>

#include <vega/Scheduler.h>
#include <vega/Promise.h>

using namespace vega;

static constexpr auto CPU_TASK_DURATION_MS = std::chrono::milliseconds(500);
static constexpr int NUM_PARALLEL_TASKS = 8;
static constexpr size_t N_WORKERS = 4;

static std::atomic<int> completedTasks {0};
static std::mutex outputMutex;

static std::map<std::thread::id, size_t> threadTaskCount;
static std::mutex threadTaskCountMutex;

static void recordThreadUsage() {
    std::lock_guard<std::mutex> lock(threadTaskCountMutex);
    threadTaskCount[std::this_thread::get_id()]++;
}


Promise<int> cpuIntensiveTask(int taskId) {
    // Encourage scheduler to dispatch task to worker thread(s).
    co_await Scheduler::getCurrent()->delay(std::chrono::milliseconds(0));

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::println("[Task {}] Starting on thread {}", taskId, std::this_thread::get_id());
    }

    recordThreadUsage();

    // simulate CPU-intensive task.
    std::this_thread::sleep_for(CPU_TASK_DURATION_MS);

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::println("[Task {}] Completed on thread {}", taskId, std::this_thread::get_id());
    }

    completedTasks++;
    co_return taskId;
}


Promise<> runParallelTasks() {
    auto startTime = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::println("Starting {} parallel CPU-intensive tasks...", NUM_PARALLEL_TASKS);
        std::println("Each task takes {} millisecond(s)", CPU_TASK_DURATION_MS.count());
        std::println("With {} worker threads, expected time: ~{} millisecond(s)", N_WORKERS, (NUM_PARALLEL_TASKS * CPU_TASK_DURATION_MS / N_WORKERS));
        std::println();
        std::cout.flush();
    }

    // Create all tasks first - they will yield to scheduler and be dispatched to worker threads.
    Promise<int> tasks[NUM_PARALLEL_TASKS];
    for (int i = 0; i < NUM_PARALLEL_TASKS; ++i) {
        tasks[i] = cpuIntensiveTask(i);
    }

    // Wait for all tasks to complete
    for (int i = 0; i < NUM_PARALLEL_TASKS; ++i) {
        int result = co_await tasks[i];

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::println("[Main] Task {} returned {}.", i, result);
            std::cout.flush();
            assert(i == result);
        }
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::println();
        std::println("All {} tasks completed in {} ms.", NUM_PARALLEL_TASKS, duration.count());
        std::cout.flush();
    }

    // make sure all tasks completed.
    assert(completedTasks == NUM_PARALLEL_TASKS);

    auto expectedMaxTime = std::chrono::milliseconds((NUM_PARALLEL_TASKS / N_WORKERS + 1) * CPU_TASK_DURATION_MS);

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::println("Expected max time: {} ms.", expectedMaxTime.count());
        std::cout.flush();
    }

    assert(duration <= expectedMaxTime);
    
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::lock_guard<std::mutex> lock2(threadTaskCountMutex);
        std::println();
        std::println("Thread usage statistics:");
        
        for (const auto& [tid, count] : threadTaskCount) {
            std::println("  Thread {}: {} task(s)", tid, count);
            std::cout.flush();
        }
    }
}


int main() {
    Scheduler{N_WORKERS}.runBlocking(runParallelTasks);
    return 0;
}
