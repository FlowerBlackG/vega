// SPDX-License-Identifier: MulanPSL-2.0


#pragma once

#include <fstream>

#include <vega/io/file/backend/FileBackend.h>


namespace vega::io {

class StreamFile : public FileBackend {
protected:
    std::fstream stream_;

public:
    StreamFile() {}

    virtual constexpr FileBackendType type() const override {
        return FileBackendType::Stream;
    }

    virtual bool open(const std::string& path, FileOpenMode mode) override;

    virtual bool isOpen() override {
        return stream_.is_open();
    }

    virtual void close() override {
        if (isOpen()) {
            stream_.close();
        }
    }

    virtual Promise<size_t> read(void* buffer, size_t size, long offset = -1) override;
    virtual Promise<size_t> write(const void* buffer, size_t size, long offset = -1) override;
};


}  // namespace vega::io
