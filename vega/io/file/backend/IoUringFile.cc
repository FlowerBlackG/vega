// SPDX-License-Identifier: MulanPSL-2.0


#if defined(__linux__)

#include <vega/io/file/backend/IoUringFile.h>

#include <fcntl.h>

#include <vega/io/IoUring.h>
#include <vega/Promise.h>
#include <vega/Scheduler.h>

namespace vega::io {


IoUring& IoUringFile::threadIoUring() {
    return Scheduler::getCurrent()->getThreadIoUring();
}


bool IoUringFile::open(const std::string& path, FileOpenMode mode) {
    int flags = 0;


    if (mode & FileOpenMode::ReadWrite)
        flags = O_RDWR;
    else if (mode & FileOpenMode::Read)
        flags = O_RDONLY;
    else if (mode & FileOpenMode::Write)
        flags = O_WRONLY;

    if (mode & FileOpenMode::Truncate)
        flags |= O_TRUNC;

    flags |= O_CREAT;
    
    
    fd_ = ::open(path.c_str(), flags, 0644);
    if (fd_ == -1) {
        return false;
    }

    return true;
}


void IoUringFile::close() {
    if (isOpen()) {
        ::close(fd_);
        fd_ = -1;
    }
}


Promise<size_t> IoUringFile::read(void* buffer, size_t size, long offset) {
    if (offset == -1)
        offset = readPos_;

    io_uring_sqe* sqe = co_await threadIoUring().getSqe();

    io_uring_prep_read(sqe, fd_, buffer, size, offset);
    auto ret = co_await threadIoUring().wait(sqe->user_data);

    if (ret.res < 0)
        throw std::runtime_error("read failed (IoUringFile)");

    readPos_ = offset + ret.res;
    
    co_return ret.res;
}


Promise<size_t> IoUringFile::write(const void* buffer, size_t size, long offset) {
    if (offset == -1)
        offset = writePos_;

    io_uring_sqe* sqe = co_await threadIoUring().getSqe();
    
    io_uring_prep_write(sqe, fd_, buffer, size, offset);

    auto ret = co_await threadIoUring().wait(sqe->user_data);

    if (ret.res < 0)
        throw std::runtime_error("read failed (IoUringFile)");

    writePos_ = offset + ret.res;
    
    co_return ret.res;
}


}  // namespace vega::io

#endif // defined(__linux__)
