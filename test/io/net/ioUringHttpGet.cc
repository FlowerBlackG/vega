// SPDX-License-Identifier: MulanPSL-2.0

#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <cassert>

#include <vega/vega.h>

using namespace vega;

// by gemini 3.0 pro.
static sockaddr_in __resolve_to_ipv4(const std::string& domain, uint16_t port) {
    struct addrinfo hints, *res;
    sockaddr_in addr{};

    // Initialize hints to look specifically for IPv4 and TCP
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Force IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // getaddrinfo returns 0 on success
    int status = getaddrinfo(domain.c_str(), nullptr, &hints, &res);
    if (status != 0) {
        throw std::runtime_error("DNS Resolution failed for " + domain + 
                                 ": " + gai_strerror(status));
    }

    // Safety check: ensure we actually got a result
    if (res == nullptr) {
        throw std::runtime_error("No IPv4 addresses found for " + domain);
    }

    // Copy the first result found
    // We know it's a sockaddr_in because we set hints.ai_family = AF_INET
    std::memcpy(&addr, res->ai_addr, sizeof(sockaddr_in));
    
    // Set the port (must be in Network Byte Order)
    addr.sin_port = htons(port);

    // Free the linked list allocated by getaddrinfo
    freeaddrinfo(res);

    return addr;
}

static io::Inet4Address resolveToIPv4(const std::string& domain, uint16_t port) {
    try {
        auto addr = __resolve_to_ipv4(domain, port);
        io::Inet4Address inetAddr;
        inetAddr.port = addr.sin_port;
        inetAddr.addr = addr.sin_addr.s_addr;
        return inetAddr;
    } catch (const std::exception& e) {
        std::cerr << "Error resolving domain: " << e.what() << std::endl;
        std::terminate();
    }
}


Promise<> suspendMain() {
    const std::string addr = "www.tencent.com";
    const uint16_t port = 80;

    const std::string httpRequest = 
        "GET / HTTP/1.1\r\nHost: " + addr + "\r\nConnection: close\r\n\r\n";
    

    auto ipv4Addr = resolveToIPv4(addr, port);
    io::IoUringInet4StreamSocket sock;
    co_await sock.connect(ipv4Addr);

    co_await sock.write(httpRequest.data(), httpRequest.size());
    
    std::vector<char> buffer(64, 0);
    size_t bytesRead = 0;

    bytesRead = co_await sock.read(buffer);
    buffer.back() = '\0';

    std::cout << "Received " << bytesRead << " bytes:\n" << buffer.data() << std::endl;

    assert(bytesRead == buffer.size());
    assert(buffer[0] != '\0');
}


int main() {
    Scheduler::getDefault().runBlocking(suspendMain);
    return 0;
}

