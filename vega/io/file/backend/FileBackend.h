// SPDX-License-Identifier: MulanPSL-2.0


#pragma once

#include <string>

#include <vega/io/file/FileOpenMode.h>


namespace vega {

template <typename T>
class Promise;

}  // namespace vega


namespace vega::io {


enum class FileBackendType {
    None = 0,
    Stream,
    IoUring,
    MemoryMap,
};

    
class FileBackend {
protected:
    long readPos_ = 0;
    long writePos_ = 0;

public:
    virtual ~FileBackend() {
        this->close();
    }

    virtual constexpr FileBackendType type() const = 0;

    long readPos() const { return readPos_; }
    long writePos() const { return writePos_; }

    virtual bool isOpen() { return false; }
    virtual bool open(const std::string&, FileOpenMode) { return false; }
    virtual void close() {}
    virtual Promise<std::size_t> read(void* buffer, std::size_t size, long offset = -1) = 0;
    virtual Promise<std::size_t> write(const void* buffer, std::size_t size, long offset = -1) = 0;
};


}  // namespace vega::io
