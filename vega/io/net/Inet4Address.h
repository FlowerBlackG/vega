// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <string>
#include <cstdint>

#ifdef _WIN32
    #include <winsock2.h>  // htons  htonl
    #include <ws2tcpip.h>  // inet_pton
#else
    #include <arpa/inet.h>
#endif


namespace vega::io {

class Inet4Address {
public:
    union {
        /** In network byte order. */
        uint32_t addr;
        uint8_t octets[4];
    };

    /** In network byte order. */
    uint16_t port;

    Inet4Address() : addr(0), port(0) {}

    Inet4Address(uint32_t addr, uint16_t port) {
        this->addr = htonl(addr);
        this->port = htons(port);
    }

    Inet4Address(const std::string& ip, uint16_t port_host) {
        port = htons(port_host);
        if (inet_pton(AF_INET, ip.c_str(), &addr) <= 0) {
            addr = 0;
            port = 0;  // mark as invalid
        }
    }


    constexpr std::string toString() const {
        std::string result;

        result += std::to_string(octets[0]);
        result += '.';
        result += std::to_string(octets[1]);
        result += '.';
        result += std::to_string(octets[2]);
        result += '.';
        result += std::to_string(octets[3]);
        result += ':';
        result += std::to_string(ntohs(port));

        return result;
    }


    constexpr operator bool () const {
        return addr != 0 || port != 0;
    }


#if defined (__linux__)
    constexpr sockaddr_in toSockAddrIn() const {
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr;
        sa.sin_port = port;
        return sa;
    }
#endif

};  // class Inet4Address

}  // namespace vega::io
