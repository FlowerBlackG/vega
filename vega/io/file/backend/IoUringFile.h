// SPDX-License-Identifier: MulanPSL-2.0


#pragma once

#if defined(__linux__)

#include <liburing.h>
#include <vega/io/file/backend/FileBackend.h>


namespace vega::io {

class IoUring;


class IoUringFile : public FileBackend {
protected:
    int fd_ = -1;

    IoUring& threadIoUring();


public:
    IoUringFile() {}

    virtual constexpr FileBackendType type() const override {
        return FileBackendType::IoUring;
    }

    virtual bool open(const std::string& path, FileOpenMode mode) override;

    virtual bool isOpen() override {
        return fd_ != -1;
    }

    virtual void close() override;

    virtual Promise<size_t> read(void* buffer, size_t size, long offset = -1) override;
    virtual Promise<size_t> write(const void* buffer, size_t size, long offset = -1) override;
};

}  // namespace vega::io

#endif // defined(__linux__)