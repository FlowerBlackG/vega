// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <memory>
#include <vector>

#include <vega/io/file/backend/FileBackend.h>


namespace vega::io {

class File {
protected:
    std::unique_ptr<FileBackend> backend_ = nullptr;

    bool createBackend();

public:
    File();
    virtual ~File();

    FileBackendType backendType() const;


    bool open(const std::string& path, FileOpenMode mode);
    bool isOpen() const;
    void close();

    Promise<std::size_t> read(void* buffer, std::size_t size, long offset = -1);
    Promise<std::size_t> read(std::vector<char>&, long offset = -1);
    Promise<std::size_t> write(const void* buffer, std::size_t size, long offset = -1);
    Promise<std::size_t> write(const std::vector<char>&, long offset = -1);

    operator bool() const;
};


}  // namespace vega::io
