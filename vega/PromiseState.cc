// SPDX-License-Identifier: MulanPSL-2.0


#include <vega/PromiseState.h>
#include <vega/Scheduler.h>

namespace vega {


void PromiseStateBase::resumeContinuationsOnScheduler(Scheduler* scheduler) {
    scheduler = scheduler ? scheduler : this->scheduler;

    if (scheduler) {
        scheduler->addTask([p = this->getPtr()] () {
            p->resumeContinuations();
        });
    }
    else {
        // resume on current thread
        this->resumeContinuations();
    }
}


}  // namespace vega
