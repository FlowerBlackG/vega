// SPDX-License-Identifier: MulanPSL-2.0


#include <vega/io/net/IoUringInet4StreamSocket.h>
#include <vega/io/net/Errors.h>

#include <vega/Scheduler.h>

#include <cstring>

#include <sys/socket.h>
#include <unistd.h>

#include <liburing.h>

namespace vega::io {


static IoUring& __ring() {
    return Scheduler::getThreadIoUring();
}


static int __tryCreateSocket() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw SocketError("Failed to create socket: " + std::string(strerror(errno)));
    }
    return fd;
}


Promise<> IoUringInet4StreamSocket::connect(const Inet4Address& remoteAddr) {
    close();
    fd_ = __tryCreateSocket();

    sockaddr_in addr = remoteAddr.toSockAddrIn();

    auto sqe = co_await __ring().getSqe();
    io_uring_prep_connect(sqe, this->fd_, (sockaddr*) &addr, sizeof(addr));
    auto res = co_await __ring().submitAndWaitRes(sqe);

    if (res < 0) {
        close();
        throw ConnectError("Failed to connect: " + std::string(strerror(-res)));
    }

    this->remoteAddr = remoteAddr;
}


Promise<> IoUringInet4StreamSocket::bind(const Inet4Address& localAddr) {
    close();
    fd_ = __tryCreateSocket();

    auto sqe = co_await __ring().getSqe();
    sockaddr_in addr = localAddr.toSockAddrIn();
    io_uring_prep_bind(sqe, this->fd_, (sockaddr*) &addr, sizeof(addr));
    auto res = co_await __ring().submitAndWaitRes(sqe);
    if (res < 0) {
        close();
        throw BindError("Failed to bind: " + std::string(strerror(-res)));
    }
    this->localAddr = localAddr;
}


void IoUringInet4StreamSocket::close() {
    if (isValid()) {
        ::close(fd_);
        fd_ = -1;
    }
}


Promise<IoUringInet4StreamSocket> IoUringInet4StreamSocket::accept() {
    auto sqe = co_await __ring().getSqe();
    io_uring_prep_accept(sqe, this->fd_, nullptr, nullptr, 0);
    auto res = co_await __ring().submitAndWaitRes(sqe);
    if (res < 0) {
        throw AcceptError("Failed to accept: " + std::string(strerror(-res)));
    }

    int clientFd = res;
    IoUringInet4StreamSocket clientSocket;
    clientSocket.fd_ = clientFd;
    co_return clientSocket;
}

Promise<std::size_t> IoUringInet4StreamSocket::readSome(void* buffer, std::size_t size) {
    auto sqe = co_await __ring().getSqe();
    io_uring_prep_read(sqe, this->fd_, buffer, size, 0);
    auto res = co_await __ring().submitAndWaitRes(sqe);
    if (res < 0) {
        throw SocketError("Failed to read: " + std::string(strerror(-res)));
    }
    co_return res;
}


Promise<std::size_t> IoUringInet4StreamSocket::writeSome(const void* buffer, std::size_t size) {
    auto sqe = co_await __ring().getSqe();
    io_uring_prep_write(sqe, this->fd_, buffer, size, 0);
    auto res = co_await __ring().submitAndWaitRes(sqe);
    if (res < 0) {
        throw SocketError("Failed to write: " + std::string(strerror(-res)));
    }
    co_return res;
}

}  // namespace vega::io
