// SPDX-License-Identifier: MulanPSL-2.0

#include <vega/Scheduler.h>
#include <vega/io/IoUring.h>

namespace vega {


/**
 * It is user's responsibility (currently) to ensure structured concurrency.
 */
static thread_local Scheduler* currentScheduler = nullptr;

/**
 * SIZE_MAX means non-worker thread (maybe a scheduler's main thread, or not belong to any scheduler).
 */
static thread_local size_t workerThreadId = SIZE_MAX;

static thread_local std::unique_ptr<io::IoUring> threadIoUring;


Scheduler::Scheduler(size_t nWorkers) : nWorkers(nWorkers > 1 ? nWorkers : 0) {
    if (this->nWorkers > 0)
        startWorkers();
}


Scheduler::~Scheduler() {
    stopAndJoinWorkers();
}


Scheduler* Scheduler::getCurrent() {
    return currentScheduler;
}



#if defined(__linux__)
size_t Scheduler::pollIoUringIfInitialized() {
    if (!threadIoUringInitialized())
        return 0;
    
    return getThreadIoUring().poll();
}
#endif


Scheduler* Scheduler::setCurrent(Scheduler* scheduler) {
    auto prev = currentScheduler;
    currentScheduler = scheduler;
    return prev;
}


bool Scheduler::isCurrentThreadWorker() const {
    return workerThreadId != SIZE_MAX;
}


bool Scheduler::isCurrentThreadMain() const {
    return currentScheduler == this && workerThreadId == SIZE_MAX;
}


bool Scheduler::shouldQueueTask() const {
    return workersStarted && workerThreadId == SIZE_MAX;
}


void Scheduler::startWorkers() {
    if (workersStarted.exchange(true)) {
        return;  // workers already started
    }

    stopWorkers = false;
    workerThreads.reserve(this->nWorkers);

    for (size_t i = 0; i < this->nWorkers; ++i) {
        workerThreads.emplace_back([this, i]() {
            this->workerThreadMain(i);
        });
    }
}


void Scheduler::stopAndJoinWorkers() {
    if (!workersStarted.exchange(false)) {
        return;  // workers not started or already stopped
    }

    stopWorkers = true;
    taskSemaphore.release(this->nWorkers);

    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
}


void Scheduler::workerThreadMain(size_t workerId) {

    Scheduler::setCurrent(this);
    workerThreadId = workerId;

    while (!stopWorkers) {
        bool semaphoreAcquired = taskSemaphore.try_acquire();

        auto ioUringTaskResolved = pollIoUringIfInitialized();
        
        if (stopWorkers)
            break;

        if (!semaphoreAcquired && !ioUringTaskResolved) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        std::optional<Task> task;
        regularTasks.withLock([&task] (auto& q) {
            if (!q.empty()) {
                task = std::move(q.front());
                q.pop();
            }
        });

        if (!task)
            continue;
            
        activeWorkers++;
        task.value()();  // note: if task throws, it will destroy the whole worker thread.
        activeWorkers--;
    }

    
    workerThreadId = SIZE_MAX;
    Scheduler::setCurrent(nullptr);
}


size_t Scheduler::dispatch() {
    size_t dispatched = 0;
    
    dispatched += dispatchDelayedTasks();

    if (!workersStarted) {
        dispatched += dispatchRegularTasks();
    }

    dispatched += pollIoUringIfInitialized();
    
    return dispatched;
}


bool Scheduler::hasPendingTasks() {
    return !regularTasks.empty() 
        || !delayedTasks.empty() 
        || !trackedPromises.empty() 
        || activeWorkers > 0;
}


size_t Scheduler::dispatchDelayedTasks() {
    auto now = std::chrono::steady_clock::now();

    size_t count = 0;

    while (true) {
        std::optional<DelayedTask> task;

        delayedTasks.withLock([&now, &task] (auto& it) {
            if (!it.empty() && it.top().resolveTime <= now) {
                task = it.top();
                it.pop();
            }
        });

        if (!task) {
            break;
        }

        task->state->resolve();
        count++;
    }

    return count;
}
    

size_t Scheduler::dispatchRegularTasks() {
    size_t count = 0;


    while (true) {
        std::optional<Scheduler::Task> task;

        regularTasks.withLock([&task] (auto& it) {
            if (!it.empty()) {
                task = std::move(it.front());
                it.pop();
            }
        });

        if (!task) {
            break;
        }

        task.value()();
        count++;
    }

    return count;
}


size_t Scheduler::removeCompletedTrackedPromises() {
    auto toBeRemoved = std::unordered_set<std::shared_ptr<PromiseStateBase>>();

    return trackedPromises.withLock([&toBeRemoved] (auto& promises) {
        for (auto it : promises) {
            if (it->status != PromiseStatus::Pending) {
                toBeRemoved.insert(it);
            }
        }

        for (auto it : toBeRemoved) {
            promises.erase(it);
        }

        return toBeRemoved.size();
    });
}
    
#if defined(__linux__)
bool Scheduler::threadIoUringInitialized() {
    return threadIoUring != nullptr;
}



io::IoUring& Scheduler::getThreadIoUring() {
    if (!threadIoUring) {
        threadIoUring = std::make_unique<io::IoUring>();
    }

    return *threadIoUring;
}
#endif

}  // namespace vega
