// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#if defined (__linux__)



#include <string>
#include <queue>
#include <stdexcept>
#include <unordered_map>


#include <liburing.h>


namespace vega {

template <typename T>
class Promise;
class PromiseStateBase;

}  // namespace vega


namespace vega::io {

const unsigned int IO_URING_QUEUE_DEPTH = 16;


struct IoUringInitError : std::runtime_error {
    IoUringInitError(const std::string& msg) : std::runtime_error(msg) {}
};


class IoUring {
public:
    struct CompleteQueueEntry {
        std::int32_t res;
        std::uint32_t flags;
    };

protected:
    io_uring ring_;
    bool initialized_ = false;

    std::queue<Promise<io_uring_sqe*>> getSqeQueue_;

    /**
     * Completed, but not waited.
     */
    std::unordered_map<std::uint64_t, CompleteQueueEntry> orphanCqes_;

    /**
     * Waited.
     */
    std::unordered_map<std::uint64_t, Promise<CompleteQueueEntry>> waitingSqes_;

    size_t drainGetSqeQueue();
    io_uring_sqe* ioUringGetSqe();

    io_uring_cqe copy(io_uring_cqe& cqe);
    io_uring_cqe copy(io_uring_cqe* cqe);

public:

    IoUring(unsigned int queueDepth = IO_URING_QUEUE_DEPTH);
    virtual ~IoUring();

    io_uring& ring() { return ring_; }

    Promise<io_uring_sqe*> getSqe();

    void submit();

    /**
     * 
     * @param sqe.user_data 
     */
    Promise<CompleteQueueEntry> wait(std::uint64_t);

    /**
     * 
     * @param sqe.user_data
     */
    Promise<int32_t> waitRes(std::uint64_t);
    

    size_t poll();

};


}  // namespace vega::io


#endif  // defined (__linux__)
