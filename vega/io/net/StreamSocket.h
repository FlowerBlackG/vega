// SPDX-License-Identifier: MulanPSL-2.0

// base class for stream sockets.

// TODO: maybe merge with vega::io::File ?

#pragma once

#include <vector>
#include <vega/Promise.h>

#include <vega/io/net/Errors.h>


namespace vega::io {


class StreamSocket {
public:
    StreamSocket() = default;
    virtual ~StreamSocket() = default;

    virtual bool isValid() const = 0;
    virtual void close() = 0;
    
    virtual Promise<std::size_t> readSome(void* buffer, std::size_t size) = 0;
    virtual Promise<std::size_t> writeSome(const void* buffer, std::size_t size) = 0;


    Promise<std::size_t> read(void* buffer, std::size_t size) {
        std::size_t totalRead = 0;
        while (totalRead < size) {
            auto n = co_await this->readSome((char*)buffer + totalRead, size - totalRead);
            if (n == 0) {
                throw SocketError("Unexpectedly read 0 bytes");
            }
            else if (n < 0) {
                throw SocketError("Failed to read: " + std::to_string(n));
            }
            totalRead += n;
        }
        co_return totalRead;
    }

    virtual Promise<std::size_t> write(const void* buffer, std::size_t size) {
        std::size_t totalWritten = 0;
        while (totalWritten < size) {
            auto n = co_await this->writeSome((const char*)buffer + totalWritten, size - totalWritten);
            if (n == 0) {
                throw SocketError("Unexpectedly wrote 0 bytes");
            }
            else if (n < 0) {
                throw SocketError("Failed to write: " + std::to_string(n));
            }
            totalWritten += n;
        }
        co_return totalWritten;
    }
    
    // -------- Convenience APIs --------

    Promise<std::size_t> read(std::vector<char>& buf) {
        return this->read(buf.data(), buf.size());
    }

    Promise<std::size_t> write(const std::vector<char>& buf) {
        return this->write(buf.data(), buf.size());
    }

    Promise<std::size_t> readSome(std::vector<char>& buf) {
        return this->readSome(buf.data(), buf.size());
    }

    Promise<std::size_t> writeSome(const std::vector<char>& buf) {
        return this->writeSome(buf.data(), buf.size());
    }

    operator bool() const { return this->isValid(); } 


};

}  // namespace vega::io
