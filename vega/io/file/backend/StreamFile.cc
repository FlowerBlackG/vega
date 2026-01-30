// SPDX-License-Identifier: MulanPSL-2.0

#include <vega/Scheduler.h>
#include <vega/Promise.h>
#include <vega/io/file/backend/StreamFile.h>

#include <filesystem>


namespace vega::io {


bool StreamFile::open(const std::string& path, FileOpenMode mode) {
    close();

    std::ios::openmode iosMode = std::ios::binary;
    
    if (mode & FileOpenMode::Read) {
        iosMode |= std::ios::in;
    }
    if (mode & FileOpenMode::Write) {
        iosMode |= std::ios::out;
    }

    if (mode & FileOpenMode::Truncate) {
        iosMode |= std::ios::trunc;
    }

    if (mode == FileOpenMode::ReadWrite && !std::filesystem::exists(path)) {
        std::ofstream { path }; // create file.
    }

    
    stream_.open(path, iosMode);

    if (stream_.is_open()) {
        stream_.seekg(0);
        stream_.seekp(0);
    }
    
    return stream_.is_open();
}



Promise<size_t> StreamFile::read(void *buffer, size_t size, long offset) {
    if (offset == -1)
        offset = this->readPos_;

    this->stream_.seekg(offset);
    
    this->stream_.read(static_cast<char*>(buffer), size);
    
    this->readPos_ = this->stream_.tellg();
        
    return Promise<size_t>::resolve(this->stream_.gcount());
}


Promise<size_t> StreamFile::write(const void *buffer, size_t size, long offset) {
    if (offset == -1)
        offset = this->writePos_;
    
    this->stream_.seekp(offset);

    auto posBefore = this->stream_.tellp();
    this->stream_.write(static_cast<const char*>(buffer), size);
    this->writePos_ = this->stream_.tellp();
        
    return Promise<size_t>::resolve(this->writePos_ - posBefore);
}


}  // namespace vega::io
