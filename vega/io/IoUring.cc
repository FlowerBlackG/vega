// SPDX-License-Identifier: MulanPSL-2.0

#if defined(__linux__)

#include <vega/io/IoUring.h>

#include <vega/Scheduler.h>
#include <vega/Promise.h>


namespace vega::io {


static thread_local uint64_t nextSqeId = 5000001ULL;


size_t IoUring::drainGetSqeQueue() {
    size_t count = 0;
    while (!getSqeQueue_.empty()) {
        io_uring_sqe* sqe = this->ioUringGetSqe();
        if (!sqe)
            break;

        auto promise = std::move(getSqeQueue_.front());
        getSqeQueue_.pop();
        count ++;
        promise.state->resolve(sqe);
    }
    return count;
}


io_uring_sqe* IoUring::ioUringGetSqe() {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe)
        return nullptr;

    sqe->user_data = nextSqeId++;
    return sqe;
}


IoUring::IoUring(unsigned int queueDepth) {
    auto errorcode = io_uring_queue_init(queueDepth, &ring_, 0);
    if (errorcode) {
        std::string msg = "io_uring_queue_init failed: " + std::to_string(errorcode);
        throw IoUringInitError(msg);
    }

    initialized_ = true;
}


IoUring::~IoUring() {
    if (!initialized_) {
        return;
    }

    io_uring_queue_exit(&ring_);

    initialized_ = false;
}


Promise<io_uring_sqe*> IoUring::getSqe() {
    io_uring_sqe* sqe = this->ioUringGetSqe();
    if (sqe)
        return Promise<io_uring_sqe*>::resolve(sqe);

    return getSqeQueue_.emplace();
}


void IoUring::submit() {
    io_uring_submit(&ring_);
}


Promise<IoUring::CompleteQueueEntry> IoUring::wait(std::uint64_t ticket) {
    if (orphanCqes_.contains(ticket)) {
        auto cqe = orphanCqes_[ticket];
        orphanCqes_.erase(ticket);
        return Promise<CompleteQueueEntry>::resolve(cqe);
    }

    if (waitingSqes_.contains(ticket)) {
        return waitingSqes_[ticket];
    }


    auto promise = (waitingSqes_[ticket] = Promise<CompleteQueueEntry>());
    Scheduler::getCurrent().track(promise);

    return promise;
}


Promise<IoUring::CompleteQueueEntry> IoUring::submitAndWait(io_uring_sqe* sqe) {
    auto promise = this->wait(sqe->user_data);
    this->submit();
    return promise;
}


Promise<int32_t> IoUring::waitRes(uint64_t userData) {
    co_return (co_await this->wait(userData)).res;
}


Promise<int32_t> IoUring::submitAndWaitRes(io_uring_sqe* sqe) {
    auto promise = this->submitAndWait(sqe);
    co_return (co_await promise).res;
}


size_t IoUring::poll() {
    size_t count = 0;

    std::vector<std::pair<CompleteQueueEntry, Promise<CompleteQueueEntry>>> promises;

    while (true) {
        io_uring_cqe* pCqe = nullptr;
        auto peekResult = io_uring_peek_cqe(&ring_, &pCqe);

        if (peekResult != 0)
            break;

        CompleteQueueEntry cqe {
            .res = pCqe->res,
            .flags = pCqe->flags,
        };
        
        std::uint64_t ticket = pCqe->user_data;

        io_uring_cqe_seen(&ring_, pCqe);


        if (waitingSqes_.contains(ticket)) {
            auto promise = waitingSqes_[ticket];
            waitingSqes_.erase(ticket);
            promises.emplace_back(cqe, promise);
        }
        else {
            orphanCqes_[ticket] = cqe;
        }

        count ++;
    }

    this->drainGetSqeQueue();

    for (auto& [cqe, promise] : promises) {
        promise.state->resolve(cqe);
    }
    
    return count;
}

IoUring& IoUring::getThreadIoUring() {
    return Scheduler::getThreadIoUring();
}


}  // namespace vega::io


#endif  // defined(__linux__)
