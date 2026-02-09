// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#if defined(__linux__)

#include <vega/io/net/Inet4StreamSocket.h>
#include <vega/io/IoUring.h>

#include <vega/Promise.h>

namespace vega::io {

class IoUringInet4StreamSocket : public Inet4StreamSocket {
protected:
    int fd_ = -1;

public:

    virtual ~IoUringInet4StreamSocket() { this->close(); }

    virtual Promise<> connect(const Inet4Address& remoteAddr) override;

    virtual Promise<> bind(const Inet4Address& localAddr) override;

    virtual bool isValid() const override { return fd_ != -1; }

    virtual Promise<IoUringInet4StreamSocket> accept();
    
    virtual void close() override;

    virtual Promise<std::size_t> readSome(void* buffer, std::size_t size) override;
    virtual Promise<std::size_t> writeSome(const void* buffer, std::size_t size) override;
};


}  // namespace vega::io


#endif
