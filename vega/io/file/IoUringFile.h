// SPDX-License-Identifier: MulanPSL-2.0


#pragma once

#if defined(__linux__)

#include <liburing.h>
#include <vega/io/file/File.h>


namespace vega::io {

class IoUring;


class IoUringFile : public File {
protected:
    int fd_ = -1;

    IoUring& threadIoUring();


public:
    IoUringFile() {}

    IoUringFile(IoUringFile&&);
    IoUringFile& operator = (IoUringFile&&);

    virtual ~IoUringFile() override { close(); }

    virtual bool open(const std::string& path, FileOpenMode mode) override;

    virtual bool isOpen() const override {
        return fd_ != -1;
    }

    virtual void close() override;

    virtual Promise<size_t> read(void* buffer, size_t size, long offset = -1) override;
    virtual Promise<size_t> write(const void* buffer, size_t size, long offset = -1) override;
};

}  // namespace vega::io

#endif // defined(__linux__)