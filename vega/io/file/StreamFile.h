// SPDX-License-Identifier: MulanPSL-2.0


#pragma once

#include <fstream>

#include <vega/io/file/File.h>


namespace vega::io {

class StreamFile : public File {
protected:
    std::fstream stream_;

public:
    StreamFile() {}

    virtual ~StreamFile() override { close(); }

    virtual bool open(const std::string& path, FileOpenMode mode) override;

    virtual bool isOpen() const override {
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
