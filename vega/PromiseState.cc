// SPDX-License-Identifier: MulanPSL-2.0


#include <vega/PromiseState.h>
#include <vega/Scheduler.h>

namespace vega {


void PromiseStateBase::resumeContinuationsOnScheduler(Scheduler* scheduler) {
    scheduler = scheduler ? scheduler : this->scheduler;

    bool shouldQueue = scheduler && (scheduler != &(Scheduler::getCurrent()) || scheduler->shouldQueueTask());

    if (shouldQueue) {
        scheduler->addTask([p = this->getPtr()] () {
            p->resumeContinuations();
        });
    }
    else {
        // fastpath: resume on current thread
        this->resumeContinuations();
    }
}


}  // namespace vega
