// SPDX-License-Identifier: MulanPSL-2.0

// base class for vega files.

#pragma once

#include <vector>

#include <vega/Promise.h>
#include <vega/io/file/FileOpenMode.h>


namespace vega::io {

class File {
protected:
    long readPos_ = 0;
    long writePos_ = 0;

public:
    File() = default;

    File(const File&) = delete;
    File& operator = (const File&) = delete;
    File(File&&) = delete;
    File& operator = (File&&) = delete;

    // for derived classes, they should call "close()" in their own destructors.
    virtual ~File() = default;

    template <typename T>
    bool is() const {
        return dynamic_cast<const T* const>(this) != nullptr;
    }

    long readPos() const { return readPos_; }
    long writePos() const { return writePos_; }


    // -------- APIs to be implemented by derived classes. --------

    virtual bool open(const std::string& path, FileOpenMode mode) = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;

    virtual Promise<std::size_t> read(void* buffer, std::size_t size, long offset = -1) = 0;
    virtual Promise<std::size_t> write(const void* buffer, std::size_t size, long offset = -1) = 0;
    
    // -------- Convenience APIs --------

    Promise<std::size_t> read(std::vector<char>& buf, long offset = -1) {
        return this->read(buf.data(), buf.size(), offset);
    }

    Promise<std::size_t> write(const std::vector<char>& buf, long offset = -1) {
        return this->write(buf.data(), buf.size(), offset);
    }

    operator bool() const { return this->isOpen(); } 
};


}  // namespace vega::io
