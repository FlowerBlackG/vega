// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <stdexcept>

#include <vega/io/net/StreamSocket.h>
#include <vega/io/net/Inet4Address.h>

#include <vega/Promise.h>

namespace vega::io {

class Inet4StreamSocket : public StreamSocket {
public:
    Inet4Address localAddr;
    Inet4Address remoteAddr;

    virtual Promise<> connect(const Inet4Address& remoteAddr) = 0;
    virtual Promise<> bind(const Inet4Address& localAddr) = 0;
};

}  // namespace vega::io

